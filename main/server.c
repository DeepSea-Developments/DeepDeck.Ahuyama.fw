
#include "server.h"
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
#define ROWS 4
#define COLS 4

static const char *TAG = "webserver";
extern xSemaphoreHandle Wifi_initSemaphore;

// create a monitor with a list of supported resolutions
// NOTE: Returns a heap allocated string, you are required to free it after use.
char *create_monitor(void)
{
	const unsigned int resolution_numbers[3][2] = {
		{1280, 720},
		{1920, 1080},
		{3840, 2160}};
	char *string = NULL;
	cJSON *name = NULL;
	cJSON *resolutions = NULL;
	cJSON *resolution = NULL;
	cJSON *width = NULL;
	cJSON *height = NULL;
	size_t index = 0;

	cJSON *monitor = cJSON_CreateObject();
	if (monitor == NULL)
	{
		cJSON_Delete(monitor);
		return string;
	}

	name = cJSON_CreateString("Awesome 4K");
	if (name == NULL)
	{
		cJSON_Delete(monitor);
		return string;
	}
	/* after creation was successful, immediately add it to the monitor,
	 * thereby transferring ownership of the pointer to it */
	cJSON_AddItemToObject(monitor, "name", name);

	resolutions = cJSON_CreateArray();
	if (resolutions == NULL)
	{
		cJSON_Delete(monitor);
		return string;
	}
	cJSON_AddItemToObject(monitor, "resolutions", resolutions);

	for (index = 0; index < (sizeof(resolution_numbers) / (2 * sizeof(int))); ++index)
	{
		resolution = cJSON_CreateObject();
		if (resolution == NULL)
		{
			cJSON_Delete(monitor);
			return string;
		}
		cJSON_AddItemToArray(resolutions, resolution);

		width = cJSON_CreateNumber(resolution_numbers[index][0]);
		if (width == NULL)
		{
			cJSON_Delete(monitor);
			return string;
		}
		cJSON_AddItemToObject(resolution, "width", width);

		height = cJSON_CreateNumber(resolution_numbers[index][1]);
		if (height == NULL)
		{
			cJSON_Delete(monitor);
			return string;
		}
		cJSON_AddItemToObject(resolution, "height", height);
	}

	string = cJSON_Print(monitor);
	if (string == NULL)
	{
		fprintf(stderr, "Failed to print monitor.\n");
	}

	cJSON_Delete(monitor);
	return string;
}

void *json_malloc(size_t size)
{
	return heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
}

void json_free(void *ptr)
{
	heap_caps_free(ptr);
}

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
			xSemaphoreGive(Wifi_initSemaphore);
		}
	}

	else
	{
		httpd_resp_set_status(req, "error");
		httpd_resp_send(req, NULL, 0);
	}

	return ESP_OK;
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

	char *string = NULL;
	cJSON *layer_data = NULL;
	cJSON *layer_name = NULL;
	cJSON *layout_pos = NULL;
	cJSON *layers = NULL;

	cJSON *monitor = cJSON_CreateObject();
	if (monitor == NULL)
	{
		httpd_resp_set_status(req, "error creting response");
		httpd_resp_send(req, NULL, 0);
		return ESP_OK;
	}

	layers = cJSON_CreateArray();
	if (layers == NULL)
	{
		cJSON_Delete(monitor);
		httpd_resp_set_status(req, "error creting response");
		httpd_resp_send(req, NULL, 0);
		return ESP_OK;
	}
	cJSON_AddItemToObject(monitor, "Layers", layers);

	for (int index = 0; index < (LAYERS); ++index)
	{
		layer_data = cJSON_CreateObject();
		if (layer_data == NULL)
		{
			cJSON_Delete(monitor);
			httpd_resp_set_status(req, "error creting response");
			httpd_resp_send(req, NULL, 0);
			return ESP_OK;
		}
		cJSON_AddItemToArray(layers, layer_data);

		layer_name = cJSON_CreateString((default_layouts[index]->name));
		if (layer_name == NULL)
		{
			cJSON_Delete(monitor);
			httpd_resp_set_status(req, "error creting response");
			httpd_resp_send(req, NULL, 0);
			return ESP_OK;
		}
		/* after creation was successful, immediately add it to the monitor,
		 * thereby transferring ownership of the pointer to it */
		char layer_key[8] = {'\0'};
		snprintf(layer_key, sizeof(layer_key), "layer_%d", index);
		cJSON_AddItemToObject(layer_data, layer_key, layer_name);

		layout_pos = cJSON_CreateNumber(index);
		if (layout_pos == NULL)
		{
			cJSON_Delete(monitor);
			httpd_resp_set_status(req, "error creting response");
			httpd_resp_send(req, NULL, 0);
			return ESP_OK;
		}
		cJSON_AddItemToObject(layer_data, "pos", layout_pos);
	}

	string = cJSON_Print(monitor);
	if (string == NULL)
	{
		fprintf(stderr, "Failed to print monitor.\n");
	}

	httpd_resp_set_type(req, "application/json");
	httpd_resp_sendstr(req, string);
	httpd_resp_set_status(req, "200");
	httpd_resp_send(req, NULL, 0);
	cJSON_Delete(monitor);
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

void fill_row(cJSON *row, char names[][10], int codes[])
{
	int i;
	cJSON *item;
	for (i = 0; i < COLS; i++)
	{
		item = cJSON_GetArrayItem(row, i);
		strcpy(names[i], cJSON_GetObjectItem(item, "name")->valuestring);
		codes[i] = cJSON_GetObjectItem(item, "key_code")->valueint;
	}
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
	int edit_layer = 0;
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

	cJSON *layer_name = cJSON_GetObjectItem(payload, "layer name");
	if (cJSON_IsString(layer_name) && (layer_name->valuestring != NULL))
	{
		printf("Layer Name = \"%s\"\n", layer_name->valuestring);
	}

	cJSON *layer_pos = cJSON_GetObjectItem(payload, "layer pos");
	if (cJSON_IsNumber(layer_pos) && (layer_name->valueint))
	{
		printf("Layer pos = \"%d\"\n", layer_pos->valueint);
	}

	for (int i = 0; i < 3; i++)
	{
		if (strcmp(default_layouts[i]->name, layer_name->valuestring) == 0)
		{
			printf("Layer[%s] found in pos=%d \n", (layer_name->valuestring), i);
			edit_layer = i;
		}
	}

	cJSON *new_layer_name = cJSON_GetObjectItem(payload, "new_name");
	if (cJSON_IsString(new_layer_name) && (new_layer_name->valuestring != NULL))
	{
		printf("New Layer Name = \"%s\"\n", new_layer_name->valuestring);
		strcpy(default_layouts[edit_layer]->name, new_layer_name->valuestring);
	}

	// // Almacenar los valores en un 2D array
	// int matrix[ROWS][COLS];
	// for (int i = 0; i < ROWS; i++)
	// {
	// 	char row_key[8] = {'\0'};
	// 	snprintf(row_key, sizeof(row_key), "row_%d", i);
	// 	cJSON *row = cJSON_GetObjectItemCaseSensitive(payload, row_key);
	// 	printf("\r\n row_%d:  \r\n", i);
	// 	if (row != NULL)
	// 	{
	// 		for (int j = 0; j < COLS; j++)
	// 		{
	// 			char col_key[4] = {'\0'};
	// 			snprintf(col_key, sizeof(col_key), "k%d", j);
	// 			cJSON *cell = cJSON_GetObjectItemCaseSensitive(row, col_key);
	// 			printf("K%d= %d  ", j, cell->valueint);
	// 			if (cell != NULL)
	// 			{
	// 				default_layouts[edit_layer]->key_map[i][j] = cell->valueint;
	// 				strcpy(default_layouts[edit_layer]->key_map_names[i][j], new_layer_name->valuestring);
	// 			}
	// 			else
	// 			{
	// 				default_layouts[edit_layer]->key_map[i][j] = KC_UNDEFINED; // Valor por defecto
	// 			}
	// 		}
	// 	}
	// 	else
	// 	{
	// 		for (int j = 0; j < COLS; j++)
	// 		{
	// 			matrix[i][j] = -1; //
	// 		}
	// 	}
	// }

	//////////////////////

	cJSON *row0 = cJSON_GetObjectItemCaseSensitive(payload, "row0");
	cJSON *row1 = cJSON_GetObjectItemCaseSensitive(payload, "row1");
	cJSON *row2 = cJSON_GetObjectItemCaseSensitive(payload, "row2");
	cJSON *row3 = cJSON_GetObjectItemCaseSensitive(payload, "row3");

	char names[ROWS][COLS][10];
	int codes[ROWS][COLS];

	fill_row(row0, names[0], codes[0]);
	fill_row(row1, names[1], codes[1]);
	fill_row(row2, names[2], codes[2]);
	fill_row(row3, names[3], codes[3]);

	int i, j;
	printf("Names:\n");
	for (i = 0; i < ROWS; i++)
	{
		for (j = 0; j < COLS; j++)
		{
			printf("%s\t", names[i][j]);			
			strcpy(default_layouts[edit_layer]->key_map_names[i][j], names[i][j]);
		}
		printf("\n");
	}

	printf("\nCodes:\n");
	for (i = 0; i < ROWS; i++)
	{
		for (j = 0; j < COLS; j++)
		{
			printf("%d\t", codes[i][j]);
			default_layouts[edit_layer]->key_map[i][j] = codes[i][j];
		}
		printf("\n");
	}

	/////////////////////

	nvs_write_layer(*default_layouts[edit_layer], layer_pos->valueint);
	nvs_load_layouts();
	cJSON_Delete(payload);
	httpd_resp_set_status(req, "200 CONFIG UPDATE");
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

esp_err_t change_keyboard_led_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "HTTP POST --> /api/led");
	int mode_t;

	char buffer[100];
	memset(&buffer, 0, sizeof(buffer));
	httpd_req_recv(req, buffer, req->content_len);
	cJSON *payload = cJSON_Parse(buffer);
	cJSON *led_mode = cJSON_GetObjectItem(payload, "led_mode");
	if (cJSON_IsNumber(led_mode))
	{
		mode_t = led_mode->valueint;
		xQueueSend(keyled_q, &mode_t, 0);
		httpd_resp_set_status(req, "200");
		httpd_resp_send(req, NULL, 0);
	}
	else
	{
		httpd_resp_set_status(req, "error");
		httpd_resp_send(req, NULL, 0);
	}

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
	config.max_uri_handlers = 20;

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

	////////LED
	httpd_uri_t change_led_color_url = {.uri = "/api/led", .method = HTTP_POST, .handler = change_keyboard_led_handler, .user_ctx = NULL};
	httpd_register_uri_handler(server, &change_led_color_url);

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
