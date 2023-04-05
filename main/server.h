#ifndef SERVER_H
#define SERVER_H

#include "esp_netif.h"
// #include <esp_http_server.h>
#include "esp_http_server.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#define MAX_APs 20
#define FILE_PATH_MAX (ESP_VFS_PATH_MAX + 128)
#define SCRATCH_BUFSIZE (1024)


extern bool myflag;
extern bool wifi_reset;
extern bool wifi_connected;
extern bool wifi_ap_mode;

void stop_webserver(httpd_handle_t server);
httpd_handle_t start_webserver(void);
esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err);
esp_err_t create_layer_url_handler(httpd_req_t *req);
esp_err_t update_layer_url_handler(httpd_req_t *req);
esp_err_t delete_layer_url_handler(httpd_req_t *req);
esp_err_t get_layerName_url_handler(httpd_req_t *req);
esp_err_t get_layer_url_handler(httpd_req_t *req);
esp_err_t connect_url_handler(httpd_req_t *req);
esp_err_t test_url_handler(httpd_req_t *req);
esp_err_t hello_get_handler(httpd_req_t *req);

#endif