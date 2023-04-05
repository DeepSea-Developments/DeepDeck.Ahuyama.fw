
#include "server.h";
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
// #include <esp_http_server.h>
#include "esp_http_server.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#include "cJSON.h"

#include "keyboard_config.h"
#include "nvs_keymaps.h"
#include "key_definitions.h"
#include "nvs_funcs.h"
// #include "mdns.h"
#include "esp_vfs.h"

static const char *TAG = "webserver";
extern xSemaphoreHandle initSemaphore;


/* An HTTP GET handler */
esp_err_t hello_get_handler(httpd_req_t *req)
{
	// esp_err_t error;
	ESP_LOGI(TAG, "hello world");
	// const char *response = (const char *)req->user_ctx;
	// error = httpd_resp_send(req, response, strlen(response));
	// if (error != ESP_OK)
	// {
	// 	ESP_LOGI(TAG, "Error %d while sending response", error);
	// }
	// else
	// 	ESP_LOGI(TAG, "response ok");

	ESP_LOGI(TAG, "hello world %s", default_layouts[0]->name);
	strcpy(default_layouts[0]->name, "new_name");
	strcpy(default_layouts[1]->name, "new_layer");
	// strcpy_s(default_layouts[0]->name,strlen("new_name"), "new_name");
	// strcpy_s(default_layouts[1]->name,strlen("new_name2"), "new_name2");

	nvs_write_layer(*default_layouts[0], 1);
	nvs_write_layer(*default_layouts[1], 1);
	ESP_LOGI(TAG, "layout1 %s", default_layouts[0]->name);
	ESP_LOGI(TAG, "layout2 %s", default_layouts[1]->name);

	// Loading layouts from nvs (if found)
	nvs_load_layouts();

	httpd_resp_set_status(req, "200");
	httpd_resp_send(req, NULL, 0);
	return ESP_OK;
}

esp_err_t test_url_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "test_url_handler");
	char buffer[100];
	memset(&buffer, 0, sizeof(buffer));
	httpd_req_recv(req, buffer, req->content_len);
	cJSON *payload = cJSON_Parse(buffer);
	cJSON *is_on_json = cJSON_GetObjectItem(payload, "is_on");
	bool is_on = cJSON_IsTrue(is_on_json);
	cJSON_Delete(payload);
	//   toggle_led(is_on);
	if (is_on)
	{
		ESP_LOGI("test_url:", "API test URL ");
		for (int i = 0; i < 3; i++)
		{
			default_layouts[0]->key_map[0][i] = KC_KP_PLUS;
		}
		nvs_write_layer(*default_layouts[0], 1);
		nvs_load_layouts();
	}

	httpd_resp_set_status(req, "204 NO CONTENT");
	httpd_resp_send(req, NULL, 0);
	return ESP_OK;
}

esp_err_t connect_url_handler(httpd_req_t *req)
{

	ESP_LOGI(TAG, "connect handler");

	char buffer[1024];
	httpd_req_recv(req, buffer, req->content_len);
	cJSON *payload = cJSON_Parse(buffer);

	if (NULL == payload)
	{
		const char *err = cJSON_GetErrorPtr();
		if (err != NULL)
		{
			ESP_LOGE(TAG, "Error parsing json before %s", err);
			cJSON_Delete(payload);

			httpd_resp_set_status(req, "500");
			return -1;
		}
	}

	cJSON *ssid = cJSON_GetObjectItem(payload, "ssid");
	cJSON *pass = cJSON_GetObjectItem(payload, "pass");

	if (cJSON_IsString(ssid) && (ssid->valuestring != NULL))
	{
		printf("WIFI SSID \"%s\"\n", ssid->valuestring);

		if (cJSON_IsString(pass) && (pass->valuestring != NULL))
		{
			printf("WIFI PASS \"%s\"\n", pass->valuestring);

			nvs_flash_init();
			nvs_handle_t nvs;
			nvs_open("wifiCreds", NVS_READWRITE, &nvs);
			nvs_set_str(nvs, "ssid", ssid->valuestring);
			nvs_set_str(nvs, "pass", pass->valuestring);
			nvs_close(nvs);
			ESP_LOGI("nvs", "Wifi Credentials Saved");
			// xTaskCreate(&resetWifi, "reset wifi", 1024 * 3, NULL, 10, NULL);
			httpd_resp_set_status(req, "200");
			httpd_resp_send(req, NULL, 0);
			// vTaskDelay(1000 / portTICK_PERIOD_MS);
			wifi_reset = true;
			xSemaphoreGive(initSemaphore);
		}
	}

	else
	{
		httpd_resp_set_status(req, "error");
		httpd_resp_send(req, NULL, 0);
	}

	return ESP_OK;
}

void *json_malloc(size_t size)
{
	return heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
}

void json_free(void *ptr)
{
	heap_caps_free(ptr);
}

/**
 * @brief Get the layer url handler object
 *
 * @param req
 * @return esp_err_t
 */
esp_err_t get_layer_url_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "HTTP GET --> /api/layers");
	httpd_resp_set_status(req, "200");

	httpd_resp_send(req, NULL, 0);

	return ESP_OK;
}
/**
 * @brief Get the layerName url handler object
 *
 * @param req
 * @return esp_err_t
 */
esp_err_t get_layerName_url_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "HTTP GET  --> /api/layers/layer_names");
	httpd_resp_set_status(req, "200");

	httpd_resp_send(req, NULL, 0);
	return ESP_OK;
}
/**
 * @brief
 *
 * @param req
 * @return esp_err_t
 */
esp_err_t delete_layer_url_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "HTTP DELETE --> /api/layers");
	httpd_resp_set_status(req, "200");

	httpd_resp_send(req, NULL, 0);
	return ESP_OK;
}
/**
 * @brief
 *
 * @param req
 * @return esp_err_t
 */
esp_err_t update_layer_url_handler(httpd_req_t *req)
{

	ESP_LOGI(TAG, "HTTP PUT --> /api/layers");

	char buffer[1024];
	// char *buffer = (char *)malloc(req->content_len);

	// if (buffer == NULL) {
	//     printf("Memory not allocated.\n");
	// 	httpd_resp_set_status(req, "500");
	//     return -1;
	// }

	// memset(&buffer, 0, sizeof(buffer));
	httpd_req_recv(req, buffer, req->content_len);
	cJSON *payload = cJSON_Parse(buffer);

	if (NULL == payload)
	{
		const char *err = cJSON_GetErrorPtr();
		if (err != NULL)
		{
			ESP_LOGE(TAG, "Error parsing json before %s", err);
			cJSON_Delete(payload);

			httpd_resp_set_status(req, "500");
			return -1;
		}
	}

	cJSON *layer_name = cJSON_GetObjectItem(payload, "layer name");
	if (cJSON_IsString(layer_name) && (layer_name->valuestring != NULL))
	{
		printf("Layer Name \"%s\"\n", layer_name->valuestring);
	}

	cJSON *row0 = cJSON_GetObjectItem(payload, "row0");
	cJSON *key0 = cJSON_GetObjectItem(row0, "k0");
	cJSON *key1 = cJSON_GetObjectItem(row0, "k1");
	cJSON *key2 = cJSON_GetObjectItem(row0, "k2");
	cJSON *key3 = cJSON_GetObjectItem(row0, "k3");
	printf("row 0 [%d %d %d %d] \r\n", key0->valueint, key1->valueint, key2->valueint, key3->valueint);

	cJSON_Delete(payload);
	httpd_resp_set_status(req, "204 NO CONTENT");
	httpd_resp_send(req, NULL, 0);
	return ESP_OK;
}
/**
 * @brief Create a layer url handler object
 *
 * @param req
 * @return esp_err_t
 */
esp_err_t create_layer_url_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "HTTP POST --> /api/layers");

	httpd_resp_set_status(req, "200");

	httpd_resp_send(req, NULL, 0);
	return ESP_OK;
}

/* This handler allows the custom error handling functionality to be
 * tested from client side. For that, when a PUT request 0 is sent to
 * URI /ctrl, the /hello and /echo URIs are unregistered and following
 * custom error handler http_404_error_handler() is registered.
 * Afterwards, when /hello or /echo is requested, this custom error
 * handler is invoked which, after sending an error message to client,
 * either closes the underlying socket (when requested URI is /echo)
 * or keeps it open (when requested URI is /hello). This allows the
 * client to infer if the custom error handler is functioning as expected
 * by observing the socket state.
 */
esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
	return ESP_OK;
	// if (strcmp("/hello", req->uri) == 0)
	// {
	// 	httpd_resp_send_err(req, HTTPD_404_NOT_FOUND,
	// 						"/hello URI is not available");
	// 	/* Return ESP_OK to keep underlying socket open */
	// 	return ESP_OK;
	// }
	// else if (strcmp("/echo", req->uri) == 0)
	// {
	// 	httpd_resp_send_err(req, HTTPD_404_NOT_FOUND,
	// 						"/echo URI is not available");
	// 	/* Return ESP_FAIL to close underlying socket */
	// 	return ESP_FAIL;
	// }
	// /* For any other URI send 404 and close socket */
	// httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Some 404 error message");
	// return ESP_FAIL;
}

httpd_handle_t start_webserver(void)
{
	httpd_handle_t server = NULL;
	httpd_config_t config = HTTPD_DEFAULT_CONFIG();

	// Start the httpd server
	ESP_ERROR_CHECK(httpd_start(&server, &config));

	ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
	// if (httpd_start(&server, &config) == ESP_OK)
	// {
	// Set URI handlers
	httpd_uri_t connect_url = {.uri = "/api/connect", .method = HTTP_POST, .handler = connect_url_handler, .user_ctx = NULL};
	// ESP_LOGI(TAG, "Registering URI handlers --> /connect");
	httpd_register_uri_handler(server, &connect_url);

	httpd_uri_t hello_url = {.uri = "/hola", .method = HTTP_GET, .handler = hello_get_handler, .user_ctx = NULL};
	// ESP_LOGI(TAG, "Registering URI handlers --> /hello");
	httpd_register_uri_handler(server, &hello_url);

	httpd_uri_t test_url = {.uri = "/api/test", .method = HTTP_POST, .handler = test_url_handler, .user_ctx = NULL};
	// ESP_LOGI(TAG, "Registering URI handlers  --> /test");
	httpd_register_uri_handler(server, &test_url);

	///////LAYERS
	httpd_uri_t get_layer_url = {.uri = "/api/layers", .method = HTTP_GET, .handler = get_layer_url_handler, .user_ctx = NULL};
	httpd_register_uri_handler(server, &get_layer_url);
	httpd_uri_t get_layerName_url = {.uri = "/api/layers/layer_names", .method = HTTP_GET, .handler = get_layerName_url_handler, .user_ctx = NULL};
	httpd_register_uri_handler(server, &get_layerName_url);
	httpd_uri_t delete_layer_url = {.uri = "/api/layers", .method = HTTP_DELETE, .handler = delete_layer_url_handler, .user_ctx = NULL};
	httpd_register_uri_handler(server, &delete_layer_url);
	httpd_uri_t create_layer_url = {.uri = "/api/layers", .method = HTTP_POST, .handler = create_layer_url_handler, .user_ctx = NULL};
	httpd_register_uri_handler(server, &create_layer_url);
	httpd_uri_t update_layer_url = {.uri = "/api/layers", .method = HTTP_PUT, .handler = update_layer_url_handler, .user_ctx = NULL};
	httpd_register_uri_handler(server, &update_layer_url);

	return server;
	// }

	// ESP_LOGI(TAG, "Error starting server!");
	// return NULL;
}
/**
 * @brief 
 * 
 * @param server 
 */
void stop_webserver(httpd_handle_t server)
{
	// Stop the httpd server
	httpd_stop(server);
}
