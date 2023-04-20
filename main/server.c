#include <string.h>
#include "server.h"
// #include <string.h>
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
#include "gesture_handles.h"
// #include "mdns.h"
#include "esp_vfs.h"
#define ROWS 4
#define COLS 4

static const char *TAG = "webserver";
extern xSemaphoreHandle Wifi_initSemaphore;

void *json_malloc(size_t size)
{
	return heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
}

void json_free(void *ptr)
{
	heap_caps_free(ptr);
}

/* An HTTP GET handler */

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

	uint8_t edit_layer = 0;
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
	if (cJSON_IsNumber(layer_pos) && (layer_pos->valueint))
	{
		printf("Layer pos = \"%d\"\n", layer_pos->valueint);
		edit_layer = layer_pos->valueint;
	}

	if (edit_layer > ((int)nvs_read_num_layers()))
	{

		httpd_resp_set_status(req, "error");
		httpd_resp_send(req, NULL, 0);
		cJSON_Delete(payload);
		return ESP_OK;
	}

	char *string = NULL;

	cJSON *layout_pos = NULL;
	cJSON *encoder_item = NULL;
	cJSON *gesture_item = NULL;
	cJSON *layer_keymap = NULL;
	cJSON *is_active = NULL;
	// uint8_t layers_num = nvs_read_num_layers();
	int index = 0;
	int index_col = 0;

	cJSON *layer_object = cJSON_CreateObject();
	if (layer_object == NULL)
	{
		httpd_resp_set_status(req, "error creting response");
		httpd_resp_send(req, NULL, 0);
		return ESP_OK;
	}

	cJSON *_name = cJSON_CreateString(key_layouts[edit_layer].name);
	cJSON_AddItemToObject(layer_object, "name", _name);

	// is_active = cJSON_CreateBool(key_layouts[index]->active);
	is_active = cJSON_CreateBool(key_layouts[edit_layer].active);
	if (is_active == NULL)
	{
		cJSON_Delete(layer_object);
		httpd_resp_set_status(req, "error creting response");
		httpd_resp_send(req, NULL, 0);
		return ESP_OK;
	}
	cJSON_AddItemToObject(layer_object, "active", is_active);

	layer_keymap = cJSON_CreateArray();
	if (layer_keymap == NULL)
	{
		cJSON_Delete(layer_object);
		httpd_resp_set_status(req, "error creting response");
		httpd_resp_send(req, NULL, 0);
		return ESP_OK;
	}
	cJSON_AddItemToObject(layer_object, "Layers", layer_keymap);

	for (index = 0; index < MATRIX_ROWS; ++index)
	{
		cJSON *layer_row = cJSON_CreateObject();
		if (layer_row == NULL)
		{
			cJSON_Delete(layer_object);
			httpd_resp_set_status(req, "error creting response");
			httpd_resp_send(req, NULL, 0);
			return ESP_OK;
		}
		cJSON_AddItemToArray(layer_keymap, layer_row);
		for (index_col = 0; index_col < MATRIX_COLS; index_col++)
		{
			// layer_name = cJSON_CreateString((key_layouts[index]->name));
			char buf_map_name[7] = {0};
			strcpy(buf_map_name, key_layouts[edit_layer].key_map_names[index][index_col]);
			layer_name = cJSON_CreateString(buf_map_name);
			if (layer_name == NULL)
			{
				cJSON_Delete(layer_object);
				httpd_resp_set_status(req, "error creting response");
				httpd_resp_send(req, NULL, 0);
				return ESP_OK;
			}
			/* after creation was successful, immediately add it to the monitor,
			 * thereby transferring ownership of the pointer to it */
			char key_name[7] = {'\0'};
			snprintf(key_name, sizeof(key_name), "name_%d", index_col);
			cJSON_AddItemToObject(layer_row, key_name, layer_name);

			layout_pos = cJSON_CreateNumber(key_layouts[edit_layer].key_map[index][index_col]);
			if (layout_pos == NULL)
			{
				cJSON_Delete(layer_object);
				httpd_resp_set_status(req, "error creting response");
				httpd_resp_send(req, NULL, 0);
				return ESP_OK;
			}
			char key_pos[7] = {'\0'};
			snprintf(key_pos, sizeof(key_name), "Key_%d", index_col);
			cJSON_AddItemToObject(layer_row, key_pos, layout_pos);
		}
	}

	cJSON *encoder_map = cJSON_CreateObject();
	cJSON_AddItemToObject(layer_object, "left encoder map", encoder_map);

	for (index = 0; index < ENCODER_SIZE; index++)
	{
		encoder_item = cJSON_CreateNumber(key_layouts[edit_layer].left_encoder_map[index]);
		if (layout_pos == NULL)
		{
			cJSON_Delete(layer_object);
			httpd_resp_set_status(req, "error creting response");
			httpd_resp_send(req, NULL, 0);
			return ESP_OK;
		}
		cJSON_AddItemToObject(encoder_map, encoder_items_names[index], encoder_item);
	}

	cJSON *r_encoder_map = cJSON_CreateObject();
	cJSON_AddItemToObject(layer_object, "right encoder map", r_encoder_map);

	for (index = 0; index < ENCODER_SIZE; index++)
	{
		encoder_item = cJSON_CreateNumber(key_layouts[edit_layer].right_encoder_map[index]);
		if (layout_pos == NULL)
		{
			cJSON_Delete(layer_object);
			httpd_resp_set_status(req, "error creting response");
			httpd_resp_send(req, NULL, 0);
			return ESP_OK;
		}
		cJSON_AddItemToObject(r_encoder_map, encoder_items_names[index], encoder_item);
	}

	cJSON *gesture_map = cJSON_CreateObject();
	cJSON_AddItemToObject(layer_object, "gesture map", gesture_map);

	for (index = 0; index < GESTURE_SIZE; index++)
	{
		gesture_item = cJSON_CreateNumber(key_layouts[edit_layer].gesture_map[index]);
		if (layout_pos == NULL)
		{
			cJSON_Delete(layer_object);
			httpd_resp_set_status(req, "error creting response");
			httpd_resp_send(req, NULL, 0);
			return ESP_OK;
		}
		cJSON_AddItemToObject(gesture_map, gesture_items_names[index], gesture_item);
	}

	string = cJSON_Print(layer_object);
	if (string == NULL)
	{
		fprintf(stderr, "Failed to print monitor.\n");
	}

	httpd_resp_set_type(req, "application/json");
	httpd_resp_sendstr(req, string);
	httpd_resp_set_status(req, "200");
	httpd_resp_send(req, NULL, 0);
	cJSON_Delete(layer_object);
	cJSON_Delete(payload);
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
	cJSON *is_active = NULL;
	uint8_t layers_num = nvs_read_num_layers();
	// int max_layers = (int) layers_num;

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

	for (int index = 0; index < ((int)layers_num); ++index)
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

		// layer_name = cJSON_CreateString((key_layouts[index]->name));
		layer_name = cJSON_CreateString((key_layouts[index].name));
		if (layer_name == NULL)
		{
			cJSON_Delete(monitor);
			httpd_resp_set_status(req, "error creting response");
			httpd_resp_send(req, NULL, 0);
			return ESP_OK;
		}
		/* after creation was successful, immediately add it to the monitor,
		 * thereby transferring ownership of the pointer to it */
		char layer_key[10] = {'\0'};
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

		// is_active = cJSON_CreateBool(key_layouts[index]->active);
		is_active = cJSON_CreateBool(key_layouts[index].active);
		if (is_active == NULL)
		{
			cJSON_Delete(monitor);
			httpd_resp_set_status(req, "error creting response");
			httpd_resp_send(req, NULL, 0);
			return ESP_OK;
		}
		cJSON_AddItemToObject(layer_data, "active", is_active);
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

	uint8_t edit_layer = 0;
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
	if (cJSON_IsNumber(layer_pos) && (layer_pos->valueint))
	{
		printf("Layer pos = \"%d\"\n", layer_pos->valueint);
		edit_layer = layer_pos->valueint;
	}

	nvs_delete_layer(edit_layer);
	current_layout = 0;
	xQueueSend(layer_recieve_q, &current_layout,
			   (TickType_t)0);
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
	dd_layer temp_layaout;

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
	if (cJSON_IsNumber(layer_pos) && (layer_pos->valueint))
	{
		printf("Layer pos = \"%d\"\n", layer_pos->valueint);
		edit_layer = layer_pos->valueint;
	}

	cJSON *new_layer_name = cJSON_GetObjectItem(payload, "new_name");
	if (cJSON_IsString(new_layer_name) && (new_layer_name->valuestring != NULL))
	{
		if (strcmp(layer_name->valuestring, key_layouts[edit_layer].name) == 0)
		{
			printf("New Layer Name = \"%s\"\n", new_layer_name->valuestring);
			strcpy(temp_layaout.name, new_layer_name->valuestring);
		}
		else
		{
			httpd_resp_set_status(req, "error layer name not match");
			httpd_resp_send(req, NULL, 0);
			return ESP_OK;
		}
	}

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

			strcpy(temp_layaout.key_map_names[i][j], names[i][j]);
		}
		printf("\n");
	}

	printf("\nCodes:\n");
	for (i = 0; i < ROWS; i++)
	{
		for (j = 0; j < COLS; j++)
		{
			printf("%d\t", codes[i][j]);

			temp_layaout.key_map[i][j] = codes[i][j];
		}
		printf("\n");
	}
	cJSON *item;
	cJSON *left_encoder_map = cJSON_GetObjectItem(payload, "left_encoder_map");
	if (left_encoder_map == NULL)
	{
		printf("No se encontró el objeto 'left_encoder_map'.\n");
		return 1;
	}
	printf("Elementos del objeto 'left_encoder_map':\n");
	cJSON_ArrayForEach(item, left_encoder_map)
	{
		printf("%s: %d\n", item->string, item->valueint);
		temp_layaout.left_encoder_map[i] = item->valueint;
		i++;
	}

	i = 0;
	cJSON *right_encoder_map = cJSON_GetObjectItem(payload, "right_encoder_map");
	cJSON_ArrayForEach(item, right_encoder_map)
	{
		printf("%s: %d\n", item->string, item->valueint);
		temp_layaout.right_encoder_map[i] = item->valueint;
		i++;
	}
	i = 0;
	cJSON *gesture_map = cJSON_GetObjectItem(payload, "gesture_map");
	cJSON_ArrayForEach(item, gesture_map)
	{
		printf("%s: %d\n", item->string, item->valueint);
		temp_layaout.gesture_map[i] = item->valueint;
		i++;
	}

	cJSON *is_active = cJSON_GetObjectItem(payload, "active");
	bool active = cJSON_IsTrue(is_active);
	if (active)
	{
		temp_layaout.active = active;
	}
	else
	{
		temp_layaout.active = false;
	}

	nvs_write_layer(temp_layaout, layer_pos->valueint);
	cJSON_Delete(payload);
	nvs_load_layouts();
	printf("..\r\n");
	// apds9960_free();
	printf("=.\r\n");
	current_layout = 0;
	xQueueSend(layer_recieve_q, &current_layout,
			   (TickType_t)0);
	// printf("x.\r\n");

	httpd_resp_set_status(req, "200");
	printf("*.\r\n");
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
	ESP_LOGI(TAG, "HTTP POST  Create Layer --> /api/layers");
	char buffer[1024];
	httpd_req_recv(req, buffer, req->content_len);
	dd_layer new_layer;
	esp_err_t res;
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

		strcpy(new_layer.name, layer_name->valuestring);
		// strcpy_s(new_layer.name, MAX_LAYOUT_NAME_LENGTH, layer_name->valuestring);

		printf("ddLayer Name = \"%s\"\n", new_layer.name);
	}

	cJSON *is_active = cJSON_GetObjectItem(payload, "active");
	bool active = cJSON_IsTrue(is_active);
	if (active)
	{
		new_layer.active = active;
	}
	else
	{
		new_layer.active = false;
	}

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
			// strcpy(key_layouts[edit_layer]->key_map_names[i][j], names[i][j]);
			strcpy(new_layer.key_map_names[i][j], names[i][j]);
		}
		printf("\n");
	}

	printf("\nCodes:\n");
	for (i = 0; i < ROWS; i++)
	{
		for (j = 0; j < COLS; j++)
		{
			printf("%d\t", codes[i][j]);
			// key_layouts[edit_layer]->key_map[i][j] = codes[i][j];
			new_layer.key_map[i][j] = codes[i][j];
		}
		printf("\n");
	}

	res = nvs_create_new_layer(new_layer);
	if (res == ESP_OK)
	{
		current_layout = 0;
		xQueueSend(layer_recieve_q, &current_layout,
				   (TickType_t)0);
		cJSON_Delete(payload);
		httpd_resp_set_status(req, "200");

		httpd_resp_send(req, NULL, 0);
	}
	else
	{
		current_layout = 0;
		xQueueSend(layer_recieve_q, &current_layout,
				   (TickType_t)0);
		cJSON_Delete(payload);
		httpd_resp_set_status(req, "error");
		httpd_resp_send(req, NULL, 0);
	}

	return ESP_OK;
}
/**
 * @brief
 *
 * @param req
 * @return esp_err_t
 */
esp_err_t restore_default_layer_url_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "HTTP POST  Restore Default Layouts --> /api/layers/restore");

	esp_err_t error = nvs_restore_default_layers();
	if (error == ESP_OK)
	{
		httpd_resp_set_status(req, "200");
		httpd_resp_send(req, NULL, 0);
	}
	else
	{
		httpd_resp_set_status(req, "error code");
		httpd_resp_send(req, NULL, 0);
	}
	current_layout = 0;
	xQueueSend(layer_recieve_q, &current_layout,
			   (TickType_t)0);

	return ESP_OK;
}
/**
 * @brief
 *
 * @param req
 * @return esp_err_t
 */
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
/**
 * @brief
 *
 * @return httpd_handle_t
 */
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

	httpd_uri_t restore_default_layer_url = {.uri = "/api/layers/restore", .method = HTTP_PUT, .handler = restore_default_layer_url_handler, .user_ctx = NULL};
	httpd_register_uri_handler(server, &restore_default_layer_url);

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
