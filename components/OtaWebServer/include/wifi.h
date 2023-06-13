/* ==============================================================
 * Programmed by: Diego Felipe Mejia Ruiz
 *                Sergio Castañeda
 * Deepsea Developments SAS.
 * Cali, Colombia
 * ==============================================================
;WiFi task header file
===============================================================*/

#ifndef MAIN_WIFI_H_
#define MAIN_WIFI_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "common.h"

#define CONFIG_MDNS_HOSTNAME        "atlantis1"
#define CO_ESP_WIFI_SSID            "Atlantis 1"
#define CO_ESP_WIFI_PASS            "Atlantis123"
#define CO_ESP_WIFI_CHANNEL         6
#define CO_MAX_STA_CONN             2

#define STACK_SIZE_TASK             4096
#define PRIORITY_WIFI_TASK          1

#define WIFI_TASK_TAG               "WIFI_T"

TaskHandle_t wifiTaskHandle;

void wifi_init();
void initWifiSoftAP(void);
void init_mDNS(void);
bool initHttpServer(void);
void stop_webserver(httpd_handle_t server);

esp_err_t get_handler(httpd_req_t *req);
esp_err_t post_handler(httpd_req_t *req);
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

#endif