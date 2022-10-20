#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#include <string.h>

#include "wifi.h"
//#include "common.h"
#include "vpp_system.h"

#define USER_ESP_WIFI_CHANNEL 1
#define USER_ESP_MAXIMUM_RETRY 3
#define USER_MAX_STA_CONN 3

static bool wifi_sta_connected = false;
static bool wifi_sta_enable = true;

/* @brief netif object for the ACCESS POINT */
static esp_netif_t *esp_netif_ap = NULL;

EventGroupHandle_t wifi_event_group;
esp_event_handler_instance_t instance_any_id;
esp_event_handler_instance_t instance_got_ip;
/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const int CONNECTED_BIT = BIT0;

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static int s_retry_num = 0;

static const char *TAG = "wifi";
uint32_t flag_station_connected = 0;
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT)
    {
        if (event_id == WIFI_EVENT_AP_STACONNECTED)
        {
            wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
            ESP_LOGI(TAG, "station " MACSTR " join, AID=%d",
                     MAC2STR(event->mac), event->aid);
            flag_station_connected = 1;
        }
        else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
        {
            wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
            ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d",
                     MAC2STR(event->mac), event->aid);
        }
        else if (event_id == WIFI_EVENT_STA_START)
        {
            esp_wifi_connect();
        }
        else if (event_id == WIFI_EVENT_STA_DISCONNECTED)
        {

            wifi_sta_connected = false;
            if (wifi_sta_enable)
            {
                if (s_retry_num < USER_ESP_MAXIMUM_RETRY)
                {
                    vTaskDelay(1000 / portTICK_PERIOD_MS);
                    // if(get_system_state(AP_MODE)) {
                    //     s_retry_num = USER_ESP_MAXIMUM_RETRY;
                    // }
                    esp_wifi_connect();
                    ESP_LOGI(TAG, "retry to connect to the AP");
                }
                else
                {
                    xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
                }
                ESP_LOGI(TAG, "connect to the AP fail");
            }
        }
    }
    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        wifi_sta_connected = true;
    }
}

void init_wifi(void)
{

    s_wifi_event_group = xEventGroupCreate();

    /* Initialize TCP/IP */
    ESP_ERROR_CHECK(esp_netif_init());

    /* Initialize the event loop */
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_event_group = xEventGroupCreate();

    /* Register our event handler for Wi-Fi, IP and Provisioning related events */
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    /* Initialize Wi-Fi including netif with default config */
    esp_netif_create_default_wifi_sta();
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
    //                                                     ESP_EVENT_ANY_ID,
    //                                                     &wifi_event_handler,
    //                                                     NULL,
    //                                                     &instance_any_id));
    // ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
    //                                                     IP_EVENT_STA_GOT_IP,
    //                                                     &wifi_event_handler,
    //                                                     NULL,
    //                                                     &instance_got_ip));
}
uint16_t scan_wifi_ap()
{
    uint16_t number = DEFAULT_SCAN_LIST_SIZE;
    wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
    uint16_t ap_count = 0;
    uint16_t n_sta = 0;
    memset(ap_info, 0, sizeof(ap_info));
    esp_wifi_scan_start(NULL, true);
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    ESP_LOGI(TAG, "Total APs scanned = %u", ap_count);
    if(ap_count >= DEFAULT_SCAN_LIST_SIZE) {
        n_sta = DEFAULT_SCAN_LIST_SIZE;
    } else {
        n_sta = ap_count;
    }
    if(wifi_list == NULL) {
        wifi_list = malloc(sizeof(Wifi_list) * n_sta);
    } else {
        wifi_list = realloc(wifi_list, sizeof(Wifi_list) * n_sta);
    }
    int j = 0;
    for (int i = 0; (i < DEFAULT_SCAN_LIST_SIZE) && (i < ap_count); i++) {
        if ((ap_info[i].authmode == WIFI_AUTH_WPA_WPA2_PSK) || (ap_info[i].authmode == WIFI_AUTH_WPA2_PSK)) {
            ESP_LOGI(TAG, "SSID \t\t%s", ap_info[i].ssid);
            strncpy(wifi_list[j].wifi_ssid, (char*)ap_info[i].ssid, MAX_AP_NAME);
            j++;
        }
    }
    return n_sta;
}
void wifi_init_softap(const char *ssid, const char *password)
{

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = "",
            .ssid_len = 0,
            .channel = USER_ESP_WIFI_CHANNEL,
            .password = "",
            .max_connection = USER_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK},
    };

    wifi_config.ap.ssid_len = strlen((const char *)wifi_config.ap.ssid);
    strcpy((char *)wifi_config.ap.ssid, ssid);
    strcpy((char *)wifi_config.ap.password, password);

    if (strlen((char *)wifi_config.ap.password) == 0)
    {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             wifi_config.ap.ssid, wifi_config.ap.password, USER_ESP_WIFI_CHANNEL);
}

esp_err_t wifi_init_sta(const char *ssid, const char *password)
{
    esp_err_t ret = ESP_FAIL;
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "",
            .password = "",
            /* Setting a password implies station will connect to all security modes including WEP/WPA.
             * However these modes are deprecated and not advisable to be used. Incase your Access point
             * doesn't support WPA2, these mode can be enabled by commenting below line */
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,

            .pmf_cfg = {
                .capable = true,
                .required = false},
        },
    };
    if(strcmp("", password) == 0) {
        wifi_config.sta.threshold.authmode = WIFI_AUTH_OPEN;
    }
    strcpy((char *)wifi_config.sta.ssid, ssid);
    strcpy((char *)wifi_config.sta.password, password);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ;

    ESP_LOGI(TAG, "wifi_init_sta finished.Connecting to %s - %s", wifi_config.sta.ssid, wifi_config.sta.password);

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           (10000 / portTICK_PERIOD_MS));

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 ssid, password);
        ret = ESP_OK;
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGE(TAG, "Failed to connect to SSID:%s, password:%s",
                 ssid, password);
    }
    else
    {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }

    return ret;
}
bool wifi_sta_is_connected(void)
{
    return wifi_sta_connected;
}
esp_err_t start_wifi_sta(const char *ssid, const char *password)
{
    ESP_LOGI(TAG, "START ESP_WIFI_MODE_STA");
    s_retry_num = 0;
    wifi_sta_enable = true;
    return wifi_init_sta(ssid, password);
}
void stop_wifi_sta(void)
{
    wifi_sta_enable = false;
    wifi_sta_connected = false;
    esp_wifi_disconnect();
    ESP_LOGI(TAG, "STOP ESP_WIFI_MODE_STA");
}
void start_wifi_ap(const char *ssid, const char *password)
{
    ESP_LOGI(TAG, "START ESP_WIFI_MODE_AP");
    wifi_init_softap(ssid, password);
}
void stop_wifi_ap(void)
{
    ESP_LOGI(TAG, "STOP ESP_WIFI_MODE_AP");
    /* The event will not be processed after unregister */
    //ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_got_ip));

    esp_event_loop_delete_default();
    esp_netif_destroy(esp_netif_ap);
    esp_wifi_stop();
    esp_wifi_deinit();
}
