/**
 * @file wifi_handles.h
 * @author Mauro (mauriciop@dsd.dev)
 * @brief
 * @version 0.1
 * @date April 2023
 *
 * @copyright Copyright (c) 2022
 *
 */
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "keymap.h"
#include "nvs_flash.h"

#include <sys/param.h>

#include "esp_netif.h"
#include "esp_http_server.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#include "wifi_handles.h"
#include "cJSON.h"
#include "menu.h"

#include "keyboard_config.h"
#include "nvs_keymaps.h"
#include "key_definitions.h"
#include "nvs_funcs.h"
#include "esp_vfs.h"
#include "server.h"
#include "esp_mac.h"

#include "mdns.h"
#include "spiffs.h"

#define MDNS_INSTANCE "DeepG Web Server"
#define MDNS_HOST_NAME "Ahuyama"

extern SemaphoreHandle_t Wifi_initSemaphore;
/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

static int s_retry_num = 0;
bool myflag = false;
bool wifi_reset = false;
bool wifi_connected = false;
bool wifi_ap_mode = false;

static const char *TAG = "wifi_handler";

static void disconnect_handler(void *arg, esp_event_base_t event_base,
							   int32_t event_id, void *event_data)
{

	httpd_handle_t *server = (httpd_handle_t *)arg;
	ESP_LOGE(TAG, "disconnect_handler");
	if (*server)
	{
		ESP_LOGI(TAG, "Stopping webserver");
		stop_webserver(*server);
		*server = NULL;
		resetWifi();
	}
}

static void connect_handler(void *arg, esp_event_base_t event_base,
							int32_t event_id, void *event_data)
{
	httpd_handle_t *server = (httpd_handle_t *)arg;
	if (*server == NULL)
	{
		ESP_LOGI(TAG, "Starting webserver");
		*server = start_webserver(CONFIG_EXAMPLE_WEB_MOUNT_POINT);
	}
}

//////////////////////////WIFI AP//////////////////////////////////////////

void wifi_AP_event_handler(void *arg, esp_event_base_t event_base,
						   int32_t event_id, void *event_data)
{
	if (event_id == WIFI_EVENT_AP_STACONNECTED)
	{
		wifi_event_ap_staconnected_t *event =
			(wifi_event_ap_staconnected_t *)event_data;
		ESP_LOGI(TAG, "station " MACSTR " join, AID=%d", MAC2STR(event->mac),
				 event->aid);
	}
	else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
	{
		wifi_event_ap_stadisconnected_t *event =
			(wifi_event_ap_stadisconnected_t *)event_data;
		ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d", MAC2STR(event->mac),
				 event->aid);
	}
}

void wifi_init_softap(void)
{
	ESP_LOGI("", "init AP Mode");
	// ESP_ERROR_CHECK(esp_netif_init());
	// ESP_ERROR_CHECK(esp_event_loop_create_default());
	esp_netif_create_default_wifi_ap();

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	ESP_ERROR_CHECK(
		esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_AP_event_handler, NULL, NULL));

	wifi_config_t wifi_config = {
		.ap = {.ssid = EXAMPLE_ESP_WIFI_SSID,
			   .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
			   .channel =
				   EXAMPLE_ESP_WIFI_CHANNEL,
			   .password = EXAMPLE_ESP_WIFI_PASS,
			   .max_connection = EXAMPLE_MAX_STA_CONN,
			   .authmode =
				   WIFI_AUTH_WPA_WPA2_PSK},
	};
	if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0)
	{
		wifi_config.ap.authmode = WIFI_AUTH_OPEN;
	}

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
	ESP_ERROR_CHECK(esp_wifi_start());

	ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
			 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS,
			 EXAMPLE_ESP_WIFI_CHANNEL);
	wifi_ap_mode = true;
	wifi_connected_oled("AP_MODE");
}
//////////////////////////////////////////////////////////////////////////

///////////////////////////WIFI STATION/////////////////////////////////////////////

void event_handler(void *arg, esp_event_base_t event_base,
				   int32_t event_id, void *event_data)
{
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
	{
		esp_wifi_connect();
	}
	else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
	{
		if (s_retry_num < ESP_STA_MAXIMUM_RETRY)
		{
			esp_wifi_connect();
			s_retry_num++;
			ESP_LOGI(TAG, "retry to connect to the AP");
		}
		else
		{
			xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
		}
		ESP_LOGI(TAG, "connect to the AP fail");
	}
	else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
	{
		ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
		ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));

		// Convierte la dirección IP a una cadena de caracteres
		char ip_char[16] = {0}; // 16 es el tamaño máximo de una dirección IP
		sprintf(ip_char, "%d.%d.%d.%d", esp_ip4_addr1_16(&event->ip_info.ip), esp_ip4_addr2_16(&event->ip_info.ip), esp_ip4_addr3_16(&event->ip_info.ip), esp_ip4_addr4_16(&event->ip_info.ip));

		wifi_connected_oled(ip_char);
		s_retry_num = 0;
		xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
	}
}

void wifi_init_sta(bool mode, char *ssid, char *pass)
{

	s_wifi_event_group = xEventGroupCreate();
	if (mode)
	{
		ESP_LOGI("wifi", "from AP to STA");
		ESP_ERROR_CHECK(esp_wifi_stop());
		ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
	}
	else
	{
		if (!wifi_connected)
		{
			ESP_LOGI("wifi", "init STA for firts time");
			esp_netif_create_default_wifi_sta();
		}
		else
		{
			ESP_LOGI("wifi", "changing network");
			ESP_ERROR_CHECK(esp_wifi_stop());
			ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
			wifi_connected = false;
		}
	}

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	esp_event_handler_instance_t instance_any_id;
	esp_event_handler_instance_t instance_got_ip;
	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
														ESP_EVENT_ANY_ID,
														&event_handler,
														NULL,
														&instance_any_id));
	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
														IP_EVENT_STA_GOT_IP,
														&event_handler,
														NULL,
														&instance_got_ip));

	wifi_config_t wifi_config = {
		.sta = {
			// .ssid =  ESP_STA_WIFI_SSID,
			// .password = ESP_STA_WIFI_PASS,
			/* Setting a password implies station will connect to all security modes including WEP/WPA.
			 * However these modes are deprecated and not advisable to be used. Incase your Access point
			 * doesn't support WPA2, these mode can be enabled by commenting below line */
			.threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
			.sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
		},
	};

	strcpy((char *)wifi_config.sta.ssid, ssid);
	strcpy((char *)wifi_config.sta.password, pass);

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
	ESP_ERROR_CHECK(esp_wifi_start());

	// ESP_LOGI(TAG, "wifi_init_sta finished.");

	/* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
	 * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
	EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
										   WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
										   pdFALSE,
										   pdFALSE,
										   portMAX_DELAY);

	/* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
	 * happened. */
	if (bits & WIFI_CONNECTED_BIT)
	{
		ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
				 ssid, pass);
		myflag = false;
		wifi_ap_mode = false;
		wifi_connected = true;
	}
	else if (bits & WIFI_FAIL_BIT)
	{
		ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
				 ssid, pass);
		myflag = true;
		wifi_connected = false;
		wifi_ap_mode = true;
	}
	else
	{
		ESP_LOGE(TAG, "UNEXPECTED EVENT");
		myflag = true;
		wifi_ap_mode = true;
	}

	/* The event will not be processed after unregister */
	ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
	ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
	vEventGroupDelete(s_wifi_event_group);
}

static char *getAuthModeName(wifi_auth_mode_t auth_mode)
{
	char *names[] = {"OPEN", "WEP", "WPA PSK", "WPA2 PSK", "WPA WPA2 PSK", "MAX"};
	return names[auth_mode];
}

void wifi_scan_sta(void)
{
	wifi_scan_config_t scan_config = {
		.ssid = 0,
		.bssid = 0,
		.channel = 0,
		.show_hidden = true};

	ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true));

	wifi_ap_record_t wifi_records[MAX_APs];

	uint16_t maxRecods = MAX_APs;
	ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&maxRecods, wifi_records));

	printf("Found %d access points:\n", maxRecods);
	printf("\n");
	printf("               SSID              | Channel | RSSI |   Auth Mode \n");
	printf("----------------------------------------------------------------\n");
	for (int i = 0; i < maxRecods; i++)
		printf("%32s | %7d | %4d | %12s\n", (char *)wifi_records[i].ssid, wifi_records[i].primary, wifi_records[i].rssi, getAuthModeName(wifi_records[i].authmode));
	printf("----------------------------------------------------------------\n");
}
///////////////////////////////////////////////////////////////////////////////////

#ifdef USE_MDNS
static void initialise_mdns(void)
{

	// initialize mDNS
	ESP_ERROR_CHECK(mdns_init());
	// set mDNS hostname (required if you want to advertise services)
	ESP_ERROR_CHECK(mdns_hostname_set(MDNS_HOST_NAME));
	ESP_LOGI(TAG, "mdns hostname set to: [%s]", MDNS_HOST_NAME);
	// set default mDNS instance name
	ESP_ERROR_CHECK(mdns_instance_name_set(MDNS_INSTANCE));

	// structure with TXT records
	mdns_txt_item_t serviceTxtData[3] = {
		{"board", "esp32"},
		{"u", "user"},
		{"p", "password"}};

	// initialize service
	ESP_ERROR_CHECK(mdns_service_add("ESP32-WebServer", "_http", "_tcp", 80, serviceTxtData, 3));

	// add another TXT item
	ESP_ERROR_CHECK(mdns_service_txt_item_set("_http", "_tcp", "path", "/foobar"));
	// change TXT item value
	ESP_ERROR_CHECK(mdns_service_txt_item_set_with_explicit_value_len("_http", "_tcp", "u", "admin", strlen("admin")));
}
#endif

void resetWifi(void)
{
	ESP_LOGW("", "resetWifi");
	vTaskDelay(1000 / portTICK_PERIOD_MS);
	ESP_ERROR_CHECK(esp_wifi_stop());
	ESP_LOGW("", "WIFI_MODE_NULL");
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
	ESP_LOGW("", "WIFI_MODE_NULL");
	xSemaphoreGive(Wifi_initSemaphore);
}

void wifiInit(void *params)
{
	init_fs();
	nvs_handle_t nvs;

	static httpd_handle_t server = NULL;
	wifi_ap_mode = false;

	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());

	ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_AP_STAIPASSIGNED, &connect_handler, &server));
	ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
	ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));

	while (true)
	{

		if (xSemaphoreTake(Wifi_initSemaphore, portMAX_DELAY))
		{
#ifdef USE_MDNS
			initialise_mdns();
#endif
			if (wifi_reset)
			{
				// server = NULL;
				wifi_reset = false;
				printf("Restarting now.\n");
				fflush(stdout);
				esp_restart();
			}

			ESP_LOGI(":", "Searching for wifi credentials");

			ESP_ERROR_CHECK(nvs_open("wifiCreds", NVS_READWRITE, &nvs));

			size_t ssidLen, passLen;
			char *ssid = NULL, *pass = NULL;

			if (nvs_get_str(nvs, "ssid", NULL, &ssidLen) == ESP_OK)
			{
				if (ssidLen > 0)
				{
					ssid = malloc(ssidLen);
					nvs_get_str(nvs, "ssid", ssid, &ssidLen);
					ESP_LOGI(":", "save ssid: %s", ssid);
				}
			}
			else
			{
				ESP_LOGI(":", "SSID NOT FOUND");
			}

			if (nvs_get_str(nvs, "pass", NULL, &passLen) == ESP_OK)
			{
				if (passLen > 0)
				{
					pass = malloc(passLen);
					nvs_get_str(nvs, "pass", pass, &passLen);
					ESP_LOGI(":", "save pass: %s", pass);
					// myflag = false;
				}
			}
			else
			{
				ESP_LOGI(":", "PASSWORD NOT FOUND");
			}

			if (ssid != NULL && pass != NULL)
			{
				// connectSTA(ssid, pass);
				wifi_init_sta(wifi_ap_mode, ssid, pass);
			}
			else
			{
				myflag = true;
			}
			if (myflag)
			{
				wifi_init_softap();
			}

			ESP_ERROR_CHECK(esp_wifi_start());
		}
		vTaskDelay(pdMS_TO_TICKS(10));
	}
}
