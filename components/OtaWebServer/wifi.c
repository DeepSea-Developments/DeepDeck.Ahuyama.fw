/* ==============================================================
 * Programmed by: Diego Felipe Mejia Ruiz
 *                Sergio Castañeda
 * Deepsea Developments SAS.
 * Cali, Colombia
 * ==============================================================
;WiFi source file
===============================================================*/

#include "../include/wifi.h"


extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_index_html_end");


/* URI handler structure for GET /uri */
httpd_uri_t uri_get = {
    .uri      = "/",
    .method   = HTTP_GET,
    .handler  = get_handler,
    .user_ctx = NULL
};

/* URI handler structure for POST /uri */
httpd_uri_t uri_post = {
    .uri      = "/update",
    .method   = HTTP_POST,
    .handler  = post_handler,
    .user_ctx = NULL
};


static void task_wifi(void* pvParameters){ 
    (void)pvParameters;

    // wait4MonitorTask();


    initWifiSoftAP();

    init_mDNS();

    initHttpServer();


    ESP_LOGI(WIFI_TASK_TAG, "Online");
    

    for(;;){
        
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}


esp_err_t get_handler(httpd_req_t *req){
    /* Send a simple response */
    httpd_resp_send(req, (const char *) index_html_start, index_html_end - index_html_start);
	return ESP_OK;
}


esp_err_t post_handler(httpd_req_t *req){

    char buf[1000];
	esp_ota_handle_t ota_handle;
	int remaining = req->content_len;

	const esp_partition_t *ota_partition = esp_ota_get_next_update_partition(NULL);
	ESP_ERROR_CHECK(esp_ota_begin(ota_partition, OTA_SIZE_UNKNOWN, &ota_handle));

	while (remaining > 0) {
		int recv_len = httpd_req_recv(req, buf, MIN(remaining, sizeof(buf)));

		// Timeout Error: Just retry
		if (recv_len == HTTPD_SOCK_ERR_TIMEOUT) {
			continue;

		// Serious Error: Abort OTA
		} else if (recv_len <= 0) {
			httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Protocol Error");
			return ESP_FAIL;
		}

		// Successful Upload: Flash firmware chunk
		if (esp_ota_write(ota_handle, (const void *)buf, recv_len) != ESP_OK) {
			httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Flash Error");
			return ESP_FAIL;
		}

		remaining -= recv_len;
	}

	// Validate and switch to new OTA image and reboot
	if (esp_ota_end(ota_handle) != ESP_OK || esp_ota_set_boot_partition(ota_partition) != ESP_OK) {
			httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Validation / Activation Error");
			return ESP_FAIL;
	}

	httpd_resp_sendstr(req, "Firmware update complete, rebooting now!\n");

	vTaskDelay(500 / portTICK_PERIOD_MS);
	esp_restart();

	return ESP_OK;
    
}


static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data){
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(WIFI_TASK_TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(WIFI_TASK_TAG, "station "MACSTR" leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}


void initWifiSoftAP(void){

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = CO_ESP_WIFI_SSID,
            .ssid_len = strlen(CO_ESP_WIFI_SSID),
            .channel = CO_ESP_WIFI_CHANNEL,
            .password = CO_ESP_WIFI_PASS,
            .max_connection = CO_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
        },
    };

    if (strlen(CO_ESP_WIFI_PASS) == 0) 
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

}


void init_mDNS(void){

    ESP_ERROR_CHECK(mdns_init());
    ESP_ERROR_CHECK(mdns_hostname_set(CONFIG_MDNS_HOSTNAME));
    ESP_ERROR_CHECK(mdns_instance_name_set("Basic HTTP Server"));

    ESP_LOGI(WIFI_TASK_TAG, "mdns hostname set to: [%s]", CONFIG_MDNS_HOSTNAME);
    
}


bool initHttpServer(void){

    /* Generate default configuration */
    static httpd_handle_t server = NULL;

    /* Empty handle to esp_http_server */
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    /* Start the httpd server */
    if (httpd_start(&server, &config) == ESP_OK) {
        /* Register URI handlers */
        httpd_register_uri_handler(server, &uri_get);
        httpd_register_uri_handler(server, &uri_post);
    }

    return server == NULL ? false : true;

}


void stop_webserver(httpd_handle_t server){
    if (server) {
        /* Stop the httpd server */
        httpd_stop(server);
    }
}


void wifi_init(void){

    wifiTaskHandle = NULL;

    if(xTaskCreate(task_wifi,                //function pointer that creates the task
                   "Wifi task",              //ASCII Task name, only for human recognition
                   STACK_SIZE_TASK,           //task size (WORD)
                   (void*)NULL,                     //init parameters
                   PRIORITY_WIFI_TASK,       //task priority
                   &wifiTaskHandle )!= pdPASS)   //task handler
    {
        for(;;){} //should no enter here
    }
}