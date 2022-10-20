#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_event.h"
#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "lwip/ip4_addr.h"
#include "lwip/dns.h"
#include "ping/ping_sock.h"
#include "freertos/event_groups.h"

#include <string.h>

#include "wifi.h"

#include "esp_sntp.h"
#include "internet.h"
#include "vpp_system.h"

#define SECONDS_FOR_CHECK_TIME 1000

static EventGroupHandle_t dns_event_group = NULL;
const int DNS_RESOLVED_BIT = BIT0;

static EventGroupHandle_t interface_ready = NULL;
const int INTERNET_INTERFACE_READY_BIT = BIT0;

static const char *TAG = "internet";

static internet_interface_t internet_interface = IOT_CLIENT_WIFI;

// static bool internet_interface_connected = false;

internet_interface_table_t internet_interface_table[] =
    {
        {.con_type = "wifi", .interface = IOT_CLIENT_WIFI}};

void SetInternetInterface(internet_interface_t interface)
{
    internet_interface = interface;
}

esp_err_t InternetInterfaceInit(void)
{
    if (!interface_ready)
    {
        interface_ready = xEventGroupCreate();
    }
    xEventGroupClearBits(interface_ready, INTERNET_INTERFACE_READY_BIT);
    switch (internet_interface)
    {
    default:
    case IOT_CLIENT_WIFI:
        init_wifi();
        break;
    }
    return ESP_OK;
}
static void initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();
}
esp_err_t obtain_time(void)
{
    esp_err_t ret = ESP_FAIL;
    time_t now = 0;
    struct tm timeinfo = {0};
    int retry = 0;
    const int retry_count = 5;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count)
    {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    if (retry != retry_count)
    {
        ret = ESP_OK;
    }
    time(&now);
    localtime_r(&now, &timeinfo);
    return ret;
}
esp_err_t get_sntp_time()
{
    esp_err_t ret = ESP_FAIL;
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    if (timeinfo.tm_year < (DSD_VALID_YEAR - 1900))
    {
        ESP_LOGI(TAG, "Time is not set yet. Connecting to internet and getting time over NTP.");
        ret = obtain_time();
        time(&now);
    }
    else
    {
        ret = ESP_OK;
    }
    return ret;
}
TaskHandle_t xtime_Handle = NULL;

void check_valid_time_task(void *pvParameter)
{
    initialize_sntp();
    while (true)
    {
        uint16_t counter = 0;
        time_t now;
        struct tm timeinfo;
        if (IsInternetInterfaceConnected())
        {
            get_sntp_time();
        }
        time(&now);
        localtime_r(&now, &timeinfo);
        while (counter < SECONDS_FOR_CHECK_TIME && (timeinfo.tm_year > (2019 - 1900)))
        {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            counter++;
        }
    }
}
esp_err_t InternetInterfaceConnect(void)
{
    char *ssid = NULL;
    char *password = NULL;
    esp_err_t ret = ESP_FAIL;
    switch (internet_interface)
    {

    default:
    case IOT_CLIENT_WIFI:

        ssid = "KURO 2s.4ETB";//config_get_str("WifiSSID");
        password = "TururuTuru";//config_get_str("WifiPass");

        ret = start_wifi_sta(ssid, password);
        // free(ssid);
        // free(password);

        break;
    }

    if (ret == ESP_OK)
    {
        xEventGroupSetBits(interface_ready, INTERNET_INTERFACE_READY_BIT);
        xTaskCreate(&check_valid_time_task, "check_valid_time_task", 1024 * 6, NULL, 5, xtime_Handle);
    }
    return ret;
}

esp_err_t InternetInterfaceDisconnect(void)
{

    switch (internet_interface)
    {
    default:
    case IOT_CLIENT_WIFI:
        stop_wifi_sta();
        break;
    }

    xEventGroupClearBits(interface_ready, INTERNET_INTERFACE_READY_BIT);
    return ESP_OK;
}

internet_interface_t GetInternetInterface(void)
{
    return internet_interface;
}

ip_addr_t ip_Addr;

void dns_found_cb(const char *name, const ip_addr_t *ipaddr, void *callback_arg)
{
    xEventGroupSetBits(dns_event_group, DNS_RESOLVED_BIT);
}

static void cmd_ping_on_ping_end(esp_ping_handle_t hdl, void *args)
{
    ip_addr_t target_addr;
    uint32_t transmitted;
    uint32_t received;
    uint32_t total_time_ms;
    esp_ping_get_profile(hdl, ESP_PING_PROF_REQUEST, &transmitted, sizeof(transmitted));
    esp_ping_get_profile(hdl, ESP_PING_PROF_REPLY, &received, sizeof(received));
    esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr));
    esp_ping_get_profile(hdl, ESP_PING_PROF_DURATION, &total_time_ms, sizeof(total_time_ms));
    // uint32_t loss = (uint32_t)((1 - ((float)received) / transmitted) * 100);
    if (IP_IS_V4(&target_addr))
    {
        // printf("\n--- %s ping statistics ---\n", inet_ntoa(*ip_2_ip4(&target_addr)));
    }
    else
    {
        //printf("\n--- %s ping statistics ---\n", inet6_ntoa(*ip_2_ip6(&target_addr)));
    }
    // printf("%d packets transmitted, %d received, %d%% packet loss, time %dms\n",
    //        transmitted, received, loss, total_time_ms);

    if (received > 0)
        xEventGroupSetBits(dns_event_group, DNS_RESOLVED_BIT);
    // delete the ping sessions, so that we clean up all resources and can create a new ping session
    // we don't have to call delete function in the callback, instead we can call delete function from other tasks
    esp_ping_delete_session(hdl);
}
static void cmd_ping_on_ping_success(esp_ping_handle_t hdl, void *args)
{
    uint8_t ttl;
    uint16_t seqno;
    uint32_t elapsed_time, recv_len;
    ip_addr_t target_addr;
    esp_ping_get_profile(hdl, ESP_PING_PROF_SEQNO, &seqno, sizeof(seqno));
    esp_ping_get_profile(hdl, ESP_PING_PROF_TTL, &ttl, sizeof(ttl));
    esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr));
    esp_ping_get_profile(hdl, ESP_PING_PROF_SIZE, &recv_len, sizeof(recv_len));
    esp_ping_get_profile(hdl, ESP_PING_PROF_TIMEGAP, &elapsed_time, sizeof(elapsed_time));
    // printf("%d bytes from %s icmp_seq=%d ttl=%d time=%d ms\n",
    //       recv_len, ipaddr_ntoa((ip_addr_t *)&target_addr), seqno, ttl, elapsed_time);
}

static void cmd_ping_on_ping_timeout(esp_ping_handle_t hdl, void *args)
{
    uint16_t seqno;
    ip_addr_t target_addr;
    esp_ping_get_profile(hdl, ESP_PING_PROF_SEQNO, &seqno, sizeof(seqno));
    esp_ping_get_profile(hdl, ESP_PING_PROF_IPADDR, &target_addr, sizeof(target_addr));
    printf("From %s icmp_seq=%d timeout\n", ipaddr_ntoa((ip_addr_t *)&target_addr), seqno);
}

void WaitInternetInterfaceReady(void)
{
    while (!interface_ready)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    xEventGroupWaitBits(interface_ready, INTERNET_INTERFACE_READY_BIT, pdFALSE, pdTRUE, portMAX_DELAY);
}
bool IsInternetInterfaceConnected(void)
{
    if (!interface_ready)
    {
        return false;
    }

    EventBits_t XuxBits;
    XuxBits = xEventGroupWaitBits(interface_ready, INTERNET_INTERFACE_READY_BIT, pdFALSE, pdTRUE, (1 / portTICK_PERIOD_MS)); // portMAX_DELAY

    if (!(XuxBits & INTERNET_INTERFACE_READY_BIT))
    {
        ESP_LOGE(TAG, "Internet Interface Not ready");
        return false;
    }

    ip_addr_t target_addr;
    memset(&target_addr, 0, sizeof(target_addr));
    esp_ping_config_t config = ESP_PING_DEFAULT_CONFIG();

    if (!dns_event_group)
    {
        dns_event_group = xEventGroupCreate();
    }

    struct addrinfo hint;
    struct addrinfo *res = NULL;
    memset(&hint, 0, sizeof(hint));
    const char *host = "www.google.com";
    /* convert ip4 string or hostname to ip4 or ip6 address */
    if (getaddrinfo(host, NULL, &hint, &res) != 0)
    {
        printf("ping: unknown host %s\n", host);
        return false;
    }
    if (res->ai_family == AF_INET)
    {
        struct in_addr addr4 = ((struct sockaddr_in *)(res->ai_addr))->sin_addr;
        inet_addr_to_ip4addr(ip_2_ip4(&target_addr), &addr4);
    }
    else
    {
        // struct in6_addr addr6 = ((struct sockaddr_in6 *)(res->ai_addr))->sin6_addr;
        // inet6_addr_to_ip6addr(ip_2_ip6(&target_addr), &addr6);
    }
    freeaddrinfo(res);

    config.target_addr = target_addr;

    /* set callback functions */
    esp_ping_callbacks_t cbs = {
        .on_ping_success = cmd_ping_on_ping_success,
        .on_ping_timeout = cmd_ping_on_ping_timeout,
        .on_ping_end = cmd_ping_on_ping_end,
        .cb_args = NULL};
    esp_ping_handle_t ping;
    esp_ping_new_session(&config, &cbs, &ping);
    esp_ping_start(ping);

    // char szURL[] = "www.google.com";

    // ESP_LOGI(TAG, "Cheking Internet with : %s", szURL);
    // dns_gethostbyname(szURL, &ip_Addr, dns_found_cb, NULL);

    EventBits_t uxBits;
    uxBits = xEventGroupWaitBits(dns_event_group, DNS_RESOLVED_BIT, pdTRUE, pdTRUE, (6000 / portTICK_PERIOD_MS)); // portMAX_DELAY
    esp_ping_stop(ping);
    esp_ping_delete_session(ping);
    if (uxBits & DNS_RESOLVED_BIT)
    {

        ESP_LOGI(TAG, "Connected to Internet");
        return true;
    }
    else
    {
        ESP_LOGE(TAG, "Not Internet Connection");
        return false;
    }

    // switch (internet_interface)
    // {
    // case IOT_CLIENT_ETHERNET:
    //     return false;
    //     break;
    // case IOT_CLIENT_GPRS:
    //     return false;
    //     break;
    // default:
    // case IOT_CLIENT_WIFI:
    //     return wifi_sta_is_connected();
    //     break;
    // }
}

void InternetInterfaceUpdate(void)
{
    char *str = NULL;
    uint8_t i;
    str = "wifi";
    for (i = 0; i < COUNT_OF(internet_interface_table); i++)
    {
        if (!strcmp(str, internet_interface_table[i].con_type))
        {
            SetInternetInterface(internet_interface_table[i].interface);
            break;
        }
    }
    if (i == COUNT_OF(internet_interface_table))
    {
        ESP_LOGE(TAG, "Error Updating from Flash , defaulting..");
        SetInternetInterface(IOT_CLIENT_WIFI);
    }
   
}
