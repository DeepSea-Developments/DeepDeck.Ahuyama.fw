/**
 * @file server.h
 * @author Mauro (mauriciop@dsd.dev)
 * @brief 
 * @version 0.1
 * @date April 2023
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef SERVER_H
#define SERVER_H

#include "esp_netif.h"
// #include <esp_http_server.h>
#include "esp_http_server.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "rgb_led.h"
#include "keyboard_config.h"

#define MAX_APs 20
#define FILE_PATH_MAX (ESP_VFS_PATH_MAX + 128)
#define SCRATCH_BUFSIZE (1024)


extern bool myflag;
extern bool wifi_reset;
extern bool wifi_connected;
extern bool wifi_ap_mode;

#define CONFIG_EXAMPLE_WEB_MOUNT_POINT "/spiffs"

void stop_webserver(httpd_handle_t server);
httpd_handle_t start_webserver(const char *base_path);
esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err);
esp_err_t create_layer_url_handler(httpd_req_t *req);
esp_err_t update_layer_url_handler(httpd_req_t *req);
esp_err_t delete_layer_url_handler(httpd_req_t *req);
esp_err_t get_layerName_url_handler(httpd_req_t *req);
esp_err_t get_layer_url_handler(httpd_req_t *req);
esp_err_t connect_url_handler(httpd_req_t *req);
esp_err_t config_url_handler(httpd_req_t *req);
esp_err_t test_url_handler(httpd_req_t *req);
esp_err_t hello_get_handler(httpd_req_t *req);

esp_err_t restore_default_layer_url_handler(httpd_req_t *req);

esp_err_t change_keyboard_led_handler(httpd_req_t *req);


esp_err_t get_macros_url_handler(httpd_req_t *req);
esp_err_t create_macro_url_handler(httpd_req_t *req);
esp_err_t delete_macro_url(httpd_req_t *req);       
esp_err_t delete_all_macro_url(httpd_req_t *req);                                            

#endif