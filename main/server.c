#include <string.h>
#include <stdlib.h>
#include "server.h"

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

#include "cJSON.h"

#include "keyboard_config.h"
#include "nvs_keymaps.h"
#include "key_definitions.h"
#include "nvs_funcs.h"
#include "gesture_handles.h"

// #include "mdns.h"
#include "esp_vfs.h"
#include "ota.h"

#define ROWS 4
#define COLS 4



extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_index_html_end");

static const char *REST_TAG = "portal-api";
static const char *TAG = "webserver";
extern xSemaphoreHandle Wifi_initSemaphore;

#define REST_CHECK(a, str, goto_tag, ...)                                              \
	do                                                                                 \
	{                                                                                  \
		if (!(a))                                                                      \
		{                                                                              \
			ESP_LOGE(REST_TAG, "%s(%d): " str, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
			goto goto_tag;                                                             \
		}                                                                              \
	} while (0)

#define CHECK_FILE_EXTENSION(filename, ext) (strcasecmp(&filename[strlen(filename) - strlen(ext)], ext) == 0)

typedef struct rest_server_context
{
	char base_path[ESP_VFS_PATH_MAX + 1];
	char scratch[SCRATCH_BUFSIZE];
} rest_server_context_t;

void *json_malloc(size_t size)
{
	return heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
}

void json_free(void *ptr)
{
	heap_caps_free(ptr);
}

void json_response(char *j_response)
{
	cJSON *response_json = cJSON_CreateObject();
	cJSON *item_reason = cJSON_CreateString("reason");
	cJSON_AddItemToObject(response_json, " TO DO ", item_reason);
	cJSON *item_message = cJSON_CreateString("message");
	cJSON_AddItemToObject(response_json, "TO DO", item_message);
	if (response_json != NULL)
		j_response = cJSON_Print(response_json);

	if (j_response == NULL)
	{
		fprintf(stderr, "Failed to print monitor.\n");
	}
	// printf("%s",j_response);
	cJSON_Delete(response_json);
}

static void json_error_generator(char *string, char *error_str)
{
	sprintf(string, "{\"error\":\"%s\"}", error_str);
}

/* An HTTP GET handler */

/**
 * @brief
 *
 * @param req
 * @return esp_err_t
 */

/*
esp_err_t connect_url_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "connect handler");

	ESP_ERROR_CHECK(httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*")); //
	// httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "*");
	httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type");
	// httpd_resp_set_hdr(req, "Access-Control-Allow-Credentials", "true");
	// httpd_resp_set_hdr(req, "Access-Control-Max-Age", "3600");
	// httpd_resp_set_hdr(req, "Access-Control-Expose-Headers", "X-Custom-Header");
	// httpd_resp_set_hdr(req, "Vary", "Origin");

	// Read the URI line and get the host
	char *string = NULL;
	char *buf;
	size_t buf_len;

	json_response(string);
	buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
	if (buf_len > 1)
	{
		buf = malloc(buf_len);
		if (httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == ESP_OK)
		{
			ESP_LOGI(TAG, "Host: %s", buf);
		}
		free(buf);
	}

	// Read the URI line and get the parameters
	buf_len = httpd_req_get_url_query_len(req) + 1;
	if (buf_len > 1)
	{
		buf = malloc(buf_len);
		nvs_flash_init();
		nvs_handle_t nvs;
		nvs_open("wifiCreds", NVS_READWRITE, &nvs);
		if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK)
		{
			ESP_LOGI(TAG, "Found URL query: %s", buf);
			char param[32];
			if (httpd_query_key_value(buf, "ssid", param, sizeof(param)) == ESP_OK)
			{
				ESP_LOGI(TAG, "Saving SSID");
				ESP_LOGI(TAG, "The string value = %s", param);
				nvs_set_str(nvs, "ssid", param);
			}
			if (httpd_query_key_value(buf, "pass", param, sizeof(param)) == ESP_OK)
			{
				ESP_LOGI(TAG, "Saving pass");
				ESP_LOGI(TAG, "The int value = %s", param);
				nvs_set_str(nvs, "pass", param);
			}
			nvs_close(nvs);
		}
		free(buf);
	}

	// The response
	ESP_LOGI(TAG, "Wifi Credentials Saved");

	httpd_resp_set_type(req, "application/json");
	httpd_resp_sendstr(req, string);
	httpd_resp_set_status(req, HTTPD_200);
	httpd_resp_send(req, NULL, 0);
	wifi_reset = true;
	xSemaphoreGive(Wifi_initSemaphore);
	return ESP_OK;
}
*/

/**
 * @brief Handler for Wifi Connection
 *
 * @param req
 * @return esp_err_t
 */
esp_err_t connect_url_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "connect handler");
	char *buffer;
	char *string = NULL;
	json_response(string);
	size_t buf_len;
	buf_len = (req->content_len) + 1;
	buffer = malloc(buf_len);
	httpd_req_recv(req, buffer, req->content_len);

	ESP_ERROR_CHECK(httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*"));
	httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type");
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

	httpd_resp_set_type(req, "application/json");
	httpd_resp_sendstr(req, string);

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
			httpd_resp_set_status(req, HTTPD_200);
			httpd_resp_send(req, NULL, 0);
		}
	}

	else
	{
		httpd_resp_set_status(req, HTTPD_400);
		httpd_resp_send(req, NULL, 0);
		free(buffer);
		return ESP_OK;
	}

	free(buffer);
	wifi_reset = true;
	xSemaphoreGive(Wifi_initSemaphore);
	return ESP_OK;
}

esp_err_t config_url_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "HTTP GET  --> /api/config");

	ESP_ERROR_CHECK(httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*")); //
	// httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "*");
	httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type");

	uint8_t layers_num = nvs_read_num_layers();
	char *string = NULL;
	cJSON *monitor = cJSON_CreateObject();
	if (monitor == NULL)
	{
		httpd_resp_set_status(req, HTTPD_400);
		httpd_resp_send(req, NULL, 0);
		return ESP_OK;
	}

	cJSON_AddItemToObject(monitor, "ssid", cJSON_CreateString("ssid"));
	cJSON_AddItemToObject(monitor, "FWVersion", cJSON_CreateString(FIRMWARE_VERSION));
	cJSON_AddItemToObject(monitor, "Mac", cJSON_CreateString("MAC"));

	string = cJSON_Print(monitor);
	if (string == NULL)
	{
		fprintf(stderr, "Failed to print monitor.\n");
	}

	httpd_resp_set_type(req, "application/json");
	httpd_resp_sendstr(req, string);
	httpd_resp_set_status(req, HTTPD_200);
	httpd_resp_send(req, NULL, 0);
	cJSON_Delete(monitor);
	return ESP_OK;
}

/**
 *
 *
 * API MACROS
 *
 *
 *
 */

/**
 * @brief Read DeepDeck macros
 *
 * @param req
 * @return esp_err_t
 */
esp_err_t get_macros_url_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "HTTP GET MACROS INFO --> /api/macros");

	ESP_ERROR_CHECK(httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*"));
	ESP_ERROR_CHECK(httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type"));

	char *string = NULL;
	int index = 0;
	cJSON *macro_data = NULL;
	cJSON *macro_name = NULL;
	cJSON *macro_keycode = NULL;
	cJSON *macro_key = NULL;

	cJSON *macro_object = cJSON_CreateObject();
	if (macro_object == NULL)
	{
		ESP_LOGI(TAG, "error creando macro_object");

		httpd_resp_set_status(req, "500");
		httpd_resp_send(req, NULL, 0);
		return ESP_OK;
	}

	cJSON *array = cJSON_CreateArray();
	if (array == NULL)
	{
		cJSON_Delete(macro_object);
		ESP_LOGI(TAG, "error creando array");
		httpd_resp_set_status(req, "500");
		httpd_resp_send(req, NULL, 0);
		return ESP_OK;
	}
	cJSON_AddItemToObject(macro_object, "macros", array);

	for (index = 0; index < total_macros; ++index)
	{
		macro_data = cJSON_CreateObject();
		if (macro_data == NULL)
		{
			ESP_LOGI(TAG, "error creando macro_data");
			cJSON_Delete(macro_object);
			httpd_resp_set_status(req, HTTPD_400);
			httpd_resp_send(req, NULL, 0);
			return ESP_OK;
		}
		cJSON_AddItemToArray(array, macro_data);

		macro_name = cJSON_CreateString((user_macros[index].name));
		if (macro_name == NULL)
		{
			ESP_LOGI(TAG, "error creando macro_name");
			cJSON_Delete(macro_object);
			httpd_resp_set_status(req, HTTPD_400);
			httpd_resp_send(req, NULL, 0);
			return ESP_OK;
		}
		cJSON_AddItemToObject(macro_data, "name", macro_name);

		macro_keycode = cJSON_CreateNumber(user_macros[index].keycode);
		if (macro_keycode == NULL)
		{
			ESP_LOGI(TAG, "error creando macro_keycode");
			cJSON_Delete(macro_object);
			httpd_resp_set_status(req, HTTPD_400);
			httpd_resp_send(req, NULL, 0);
			return ESP_OK;
		}
		cJSON_AddItemToObject(macro_data, "keycode", macro_keycode);

		macro_key = cJSON_CreateArray();
		if (macro_key == NULL)
		{
			ESP_LOGI(TAG, "error creando macro_key");
			cJSON_Delete(macro_object);
			httpd_resp_set_status(req, HTTPD_400);
			httpd_resp_send(req, NULL, 0);
			return ESP_OK;
		}
		cJSON_AddItemToObject(macro_data, "key", macro_key);
		for (int i = 0; i < MACRO_LEN; i++)
		{
			cJSON_AddItemToArray(macro_key, cJSON_CreateNumber(user_macros[index].key[i]));
		}
	}

	string = cJSON_Print(macro_object);
	if (string == NULL)
	{
		ESP_LOGE(TAG, "cJSON_Print(macro_object) equals NULL");
	}

	httpd_resp_set_type(req, "application/json");
	httpd_resp_sendstr(req, string);
	httpd_resp_set_status(req, HTTPD_200);
	httpd_resp_send(req, NULL, 0);
	cJSON_Delete(macro_object);

	return ESP_OK;
}

/**
 * @brief Create new macro
 *
 * @param req
 * @return esp_err_t
 */
esp_err_t create_macro_url_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "HTTP POST CREATE MACRO --> /api/macros");
	ESP_ERROR_CHECK(httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*"));
	ESP_ERROR_CHECK(httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type"));

	char *buf;
	size_t buf_len;
	char *string = NULL;
	json_response(string);

	buf_len = (req->content_len) + 1;
	buf = malloc(buf_len);
	httpd_req_recv(req, buf, req->content_len);

	dd_macros new_macro;
	cJSON *payload = cJSON_Parse(buf);

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

	cJSON *name = cJSON_GetObjectItem(payload, "name");
	if (cJSON_IsString(name) && (name->valuestring != NULL))
	{
		strcpy(new_macro.name, name->valuestring);
	}
	cJSON *keycode = cJSON_GetObjectItem(payload, "keycode");
	if (cJSON_IsNumber(keycode))
	{
		new_macro.keycode = keycode->valueint;
	}
	cJSON *key = cJSON_GetObjectItem(payload, "key");
	if (cJSON_IsArray(key))
	{
		for (int i = 0; i < MACRO_LEN; i++)
		{
			cJSON *item = cJSON_GetArrayItem(key, i);
			if (cJSON_IsNumber(item))
			{
				new_macro.key[i] = item->valueint;
			}
		}
	}

	ESP_LOGI(TAG, "new_macro.name: %s, new_macro.keycode: %d, new_macro.key:{%d,%d,%d,%d,%d} ", new_macro.name, new_macro.keycode, new_macro.key[0], new_macro.key[1], new_macro.key[2], new_macro.key[3], new_macro.key[4]);

	esp_err_t error;
	error = nvs_create_new_macro(new_macro);
	if (error != ESP_OK)
	{
		ESP_LOGE(TAG, "nvs_create_new_macro error %s", esp_err_to_name(error));
		httpd_resp_set_status(req, HTTPD_400);
	}
	else
	{

		httpd_resp_set_status(req, HTTPD_200);
	}
	cJSON_Delete(payload);
	free(buf);
	httpd_resp_set_type(req, "application/json");
	httpd_resp_sendstr(req, string);
	httpd_resp_send(req, NULL, 0);
	return ESP_OK;
}

/**
 * @brief Delete a macro
 *
 * @param req
 * @return esp_err_t
 */
esp_err_t delete_macro_url_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "HTTP DELETE MACRO --> /api/macros");
	ESP_ERROR_CHECK(httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*"));
	ESP_ERROR_CHECK(httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type"));
	char *string = NULL;
	json_response(string);

	httpd_resp_set_type(req, "application/json");
	httpd_resp_sendstr(req, string);
	httpd_resp_set_status(req, HTTPD_200);
	httpd_resp_send(req, NULL, 0);
	return ESP_OK;
}

/**
 * @brief Update a macro url handler object
 *
 * @param req
 * @return esp_err_t
 */
esp_err_t update_macro_url_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "HTTP UPDATE MACRO --> /api/macros");
	ESP_ERROR_CHECK(httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*"));
	ESP_ERROR_CHECK(httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type"));

	char *buf;
	size_t buf_len;
	char *string = NULL;
	json_response(string);

	buf_len = (req->content_len) + 1;
	buf = malloc(buf_len);
	httpd_req_recv(req, buf, req->content_len);

	dd_macros new_macro;
	cJSON *payload = cJSON_Parse(buf);

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

	cJSON *name = cJSON_GetObjectItem(payload, "name");
	if (cJSON_IsString(name) && (name->valuestring != NULL))
	{
		strcpy(new_macro.name, name->valuestring);
	}
	cJSON *keycode = cJSON_GetObjectItem(payload, "keycode");
	if (cJSON_IsNumber(keycode))
	{
		new_macro.keycode = keycode->valueint;
	}
	cJSON *key = cJSON_GetObjectItem(payload, "key");
	if (cJSON_IsArray(key))
	{
		for (int i = 0; i < MACRO_LEN; i++)
		{
			cJSON *item = cJSON_GetArrayItem(key, i);
			if (cJSON_IsNumber(item))
			{
				new_macro.key[i] = item->valueint;
			}
		}
	}

	ESP_LOGI(TAG, "new_macro.name: %s, new_macro.keycode: %d, new_macro.key:{%d,%d,%d,%d,%d} ", new_macro.name, new_macro.keycode, new_macro.key[0], new_macro.key[1], new_macro.key[2], new_macro.key[3], new_macro.key[4]);
	esp_err_t error;
	error = nvs_update_macro(new_macro);
	if (error != ESP_OK)
	{
		ESP_LOGE(TAG, "nvs_update_macro error %s", esp_err_to_name(error));
		httpd_resp_set_status(req, HTTPD_400);
	}
	else
	{

		httpd_resp_set_status(req, HTTPD_200);
	}
	httpd_resp_set_type(req, "application/json");
	httpd_resp_sendstr(req, string);
	httpd_resp_send(req, NULL, 0);
	cJSON_Delete(payload);
	free(buf);

	return ESP_OK;
}

/**
 * @brief Restore default macros
 *
 * @param req
 * @return esp_err_t
 */
esp_err_t restore_default_macro_url_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "HTTP RESTORE DEFAULT MACROS --> /api/macros/restore");
	ESP_ERROR_CHECK(httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*")); //
	httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type");

	char *string = NULL;
	json_response(string);
	httpd_resp_set_type(req, "application/json");
	httpd_resp_sendstr(req, string);
	esp_err_t error = nvs_restore_default_macros();
	if (error == ESP_OK)
	{
		httpd_resp_set_status(req, HTTPD_200);
	}
	else
	{
		httpd_resp_set_status(req, HTTPD_400);
	}
	httpd_resp_send(req, NULL, 0);

	return ESP_OK;
}

/**
 * API LAYERS
 *
 */

/**
 * @brief Get the layer url handler object
 *
 * @param req
 * @return esp_err_t
 */
esp_err_t get_layer_url_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "HTTP GET LAYER INFO --> /api/layers");

	ESP_ERROR_CHECK(httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*"));
	httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type");

	// Read the URI line and get the host
	char *buf;
	size_t buf_len;
	char uuid_param[SHORT_UUID_STR_LEN];
	int position = 0;

	// Read the URI line and get the parameters
	buf_len = httpd_req_get_url_query_len(req) + 1;
	if (buf_len > 1)
	{
		buf = malloc(buf_len);
		if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK)
		{
			ESP_LOGI(TAG, "Found URL query: %s", buf);

			if (httpd_query_key_value(buf, "uuid", uuid_param, sizeof(uuid_param)) == ESP_OK)
			{
				ESP_LOGI(TAG, "The string value = %s", uuid_param);
			}
			else
			{
				ESP_LOGE(TAG, "There is no uuid parameter in the query");
				httpd_resp_set_status(req, HTTPD_400);
				httpd_resp_send(req, NULL, 0);
				return ESP_OK;
			}
		}
		free(buf);
	}
	else
	{
		ESP_LOGE(TAG, "There is no uuid parameter in the query");

		httpd_resp_set_status(req, HTTPD_400);
		httpd_resp_send(req, NULL, 0);
		return ESP_OK;
	}

	int found_flag = 0;
	int pos = 0;
	for (pos = 0; pos < layers_num; pos++)
	{
		if (strcmp(key_layouts[pos].uuid_str, uuid_param) == 0) // If both are equal
		{
			found_flag = 1; // activate found flag
			break;			// Get off the for loop
		}
	}
	if (!found_flag) // IF not found, return http error
	{
		ESP_LOGE(TAG, "UUID not found");
		httpd_resp_set_status(req, HTTPD_400);
		httpd_resp_send(req, NULL, 0);
		return ESP_OK;
	}

	char *string = NULL;
	cJSON *encoder_item = NULL;
	cJSON *gesture_item = NULL;
	cJSON *is_active = NULL;
	int index = 0;
	int index_col = 0;

	cJSON *layer_object = cJSON_CreateObject();
	if (layer_object == NULL)
	{
		ESP_LOGI(TAG, "error creando layer_object");

		httpd_resp_set_status(req, "500");
		httpd_resp_send(req, NULL, 0);
		return ESP_OK;
	}

	cJSON *_name = cJSON_CreateString(key_layouts[pos].name);
	cJSON_AddItemToObject(layer_object, "name", _name);

	// is_active = cJSON_CreateBool(key_layouts[index]->active);
	is_active = cJSON_CreateBool(key_layouts[pos].active);
	if (is_active == NULL)
	{
		cJSON_Delete(layer_object);
		httpd_resp_set_status(req, HTTPD_400);
		httpd_resp_send(req, NULL, 0);
		return ESP_OK;
	}
	cJSON_AddItemToObject(layer_object, "active", is_active);

	for (index = 0; index < MATRIX_ROWS; ++index)
	{
		char key_name[7] = {'\0'};
		snprintf(key_name, sizeof(key_name), "row%d", index);
		cJSON *row = cJSON_CreateArray();
		if (row == NULL)
		{
			cJSON_Delete(layer_object);
			httpd_resp_set_status(req, HTTPD_400);
			httpd_resp_send(req, NULL, 0);
			return ESP_OK;
		}
		cJSON_AddItemToObject(layer_object, key_name, row);
		for (index_col = 0; index_col < MATRIX_COLS; index_col++)
		{
			cJSON *key = cJSON_CreateObject();
			cJSON_AddStringToObject(key, "name", key_layouts[pos].key_map_names[index][index_col]);
			cJSON_AddNumberToObject(key, "key_code", key_layouts[pos].key_map[index][index_col]);
			cJSON_AddItemToArray(row, key);
		}
	}

	cJSON *encoder_map = cJSON_CreateObject();
	cJSON_AddItemToObject(layer_object, "left_encoder_map", encoder_map);

	for (index = 0; index < ENCODER_SIZE; index++)
	{
		encoder_item = cJSON_CreateNumber(key_layouts[pos].left_encoder_map[index]);
		if (encoder_item == NULL)
		{
			cJSON_Delete(layer_object);
			httpd_resp_set_status(req, HTTPD_400);
			httpd_resp_send(req, NULL, 0);
			return ESP_OK;
		}
		cJSON_AddItemToObject(encoder_map, encoder_items_names[index], encoder_item);
	}

	cJSON *r_encoder_map = cJSON_CreateObject();
	cJSON_AddItemToObject(layer_object, "right_encoder_map", r_encoder_map);

	for (index = 0; index < ENCODER_SIZE; index++)
	{
		encoder_item = cJSON_CreateNumber(key_layouts[pos].right_encoder_map[index]);
		if (encoder_item == NULL)
		{
			cJSON_Delete(layer_object);
			httpd_resp_set_status(req, HTTPD_400);
			httpd_resp_send(req, NULL, 0);
			return ESP_OK;
		}
		cJSON_AddItemToObject(r_encoder_map, encoder_items_names[index], encoder_item);
	}

	cJSON *gesture_map = cJSON_CreateObject();
	cJSON_AddItemToObject(layer_object, "gesture_map", gesture_map);

	for (index = 0; index < GESTURE_SIZE; index++)
	{
		gesture_item = cJSON_CreateNumber(key_layouts[pos].gesture_map[index]);
		if (gesture_item == NULL)
		{

			cJSON_Delete(layer_object);
			httpd_resp_set_status(req, HTTPD_400);
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
	httpd_resp_set_status(req, HTTPD_200);
	httpd_resp_send(req, NULL, 0);
	cJSON_Delete(layer_object);

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
	esp_log_level_set(TAG, ESP_LOG_DEBUG);
	ESP_LOGI(TAG, "HTTP GET  --> /api/layers/layer_names");

	ESP_ERROR_CHECK(httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*")); //
	// httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "*");
	httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type");
	// httpd_resp_set_hdr(req, "Access-Control-Allow-Credentials", "true");
	// httpd_resp_set_hdr(req, "Access-Control-Max-Age", "3600");
	// httpd_resp_set_hdr(req, "Access-Control-Expose-Headers", "X-Custom-Header");
	// httpd_resp_set_hdr(req, "Vary", "Origin");

	uint8_t layers_num = nvs_read_num_layers();
	char *string = NULL;
	cJSON *layer_data = NULL;
	cJSON *layer_name = NULL;
	cJSON *layout_pos = NULL;
	cJSON *layers = NULL;
	cJSON *is_active = NULL;
	cJSON *layout_uuid = NULL;
	cJSON *monitor = cJSON_CreateObject();
	if (monitor == NULL)
	{
		httpd_resp_set_status(req, HTTPD_400);
		httpd_resp_send(req, NULL, 0);
		return ESP_OK;
	}

	layers = cJSON_CreateArray();
	if (layers == NULL)
	{
		cJSON_Delete(monitor);
		httpd_resp_set_status(req, HTTPD_400);
		httpd_resp_send(req, NULL, 0);
		return ESP_OK;
	}
	cJSON_AddItemToObject(monitor, "data", layers);

	for (int index = 0; index < ((int)layers_num); ++index)
	{
		layer_data = cJSON_CreateObject();
		if (layer_data == NULL)
		{
			cJSON_Delete(monitor);
			httpd_resp_set_status(req, HTTPD_400);
			httpd_resp_send(req, NULL, 0);
			return ESP_OK;
		}
		cJSON_AddItemToArray(layers, layer_data);

		// layer_name = cJSON_CreateString((key_layouts[index]->name));
		layer_name = cJSON_CreateString((key_layouts[index].name));
		if (layer_name == NULL)
		{
			cJSON_Delete(monitor);
			httpd_resp_set_status(req, HTTPD_400);
			httpd_resp_send(req, NULL, 0);
			return ESP_OK;
		}
		/* after creation was successful, immediately add it to the monitor,
		 * thereby transferring ownership of the pointer to it */
		cJSON_AddItemToObject(layer_data, "name", layer_name);

		layout_pos = cJSON_CreateNumber(index);
		if (layout_pos == NULL)
		{
			cJSON_Delete(monitor);
			httpd_resp_set_status(req, HTTPD_400);
			httpd_resp_send(req, NULL, 0);
			return ESP_OK;
		}
		cJSON_AddItemToObject(layer_data, "pos", layout_pos);

		// is_active = cJSON_CreateBool(key_layouts[index]->active);
		is_active = cJSON_CreateBool(key_layouts[index].active);
		if (is_active == NULL)
		{
			cJSON_Delete(monitor);
			httpd_resp_set_status(req, HTTPD_400);
			httpd_resp_send(req, NULL, 0);
			return ESP_OK;
		}
		cJSON_AddItemToObject(layer_data, "active", is_active);

		layout_uuid = cJSON_CreateString((key_layouts[index].uuid_str));
		if (layout_uuid == NULL)
		{
			cJSON_Delete(monitor);
			httpd_resp_set_status(req, HTTPD_400);
			httpd_resp_send(req, NULL, 0);
			return ESP_OK;
		}
		/* after creation was successful, immediately add it to the monitor,
		 * thereby transferring ownership of the pointer to it */
		// char layer_key[10] = {'\0'};
		// snprintf(layer_key, 6, "uuid", index);
		cJSON_AddItemToObject(layer_data, "uuid", layout_uuid);
	}

	string = cJSON_Print(monitor);
	if (string == NULL)
	{
		fprintf(stderr, "Failed to print monitor.\n");
	}

	httpd_resp_set_type(req, "application/json");
	httpd_resp_sendstr(req, string);
	httpd_resp_set_status(req, HTTPD_200);
	httpd_resp_send(req, NULL, 0);
	cJSON_Delete(monitor);
	return ESP_OK;
}

/**
 * @brief Delete layer
 *
 * @param req
 * @return esp_err_t
 */
esp_err_t delete_layer_url_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "HTTP DELETE LAYER --> /api/layers");
	ESP_ERROR_CHECK(httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*")); //
	httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type");

	char string[100];

	// Read the URI line and get the host
	char *buf;
	size_t buf_len;

	char uuid_param[SHORT_UUID_STR_LEN];

	uint8_t layers_num = nvs_read_num_layers();

	if (layers_num == 1)
	{
		ESP_LOGE(TAG, "Only one layer. Cannot delete.");
		httpd_resp_set_status(req, HTTPD_500);
		httpd_resp_send(req, NULL, 0);
		return ESP_OK;
	}

	// Read the URI line and get the parameters
	buf_len = httpd_req_get_url_query_len(req) + 1;
	if (buf_len > 1)
	{
		buf = malloc(buf_len);
		if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK)
		{
			ESP_LOGI(TAG, "Found URL query: %s", buf);

			if (httpd_query_key_value(buf, "uuid", uuid_param, sizeof(uuid_param)) == ESP_OK)
			{
				ESP_LOGI(TAG, "The string value = %s", uuid_param);
			}
			else
			{
				ESP_LOGE(TAG, "There is no uuid parameter in the query");

				// ToDo - Tested this but did not work. have to check more.
				// json_error_generator(string,"There is no uuid parameter in the query");
				// httpd_resp_set_type(req, "application/json");
				// httpd_resp_sendstr(req, string);

				httpd_resp_set_status(req, HTTPD_500);
				httpd_resp_send(req, NULL, 0);
				return ESP_OK;
			}
		}
		else
		{
			ESP_LOGE(TAG, "There is no uuid parameter in the query");
			httpd_resp_set_status(req, HTTPD_500);
			httpd_resp_send(req, NULL, 0);
			return ESP_OK;
		}
		free(buf);
	}

	int found_flag = 0;
	int pos = 0;
	for (pos = 0; pos < layers_num; pos++)
	{
		if (strcmp(key_layouts[pos].uuid_str, uuid_param) == 0) // If both are equal
		{
			found_flag = 1; // activate found flag
			break;			// Get off the for loop
		}
	}
	if (!found_flag) // IF not found, return http error
	{
		ESP_LOGE(TAG, "UUID of element to delete not found");
		httpd_resp_set_status(req, HTTPD_400);
		httpd_resp_send(req, NULL, 0);
		return ESP_OK;
	}

	nvs_delete_layer(pos); // Deletes the layer found

	httpd_resp_set_status(req, HTTPD_200);
	httpd_resp_send(req, NULL, 0);

	current_layout = 0;
	xQueueSend(layer_recieve_q, &current_layout,
			   (TickType_t)0);

	return ESP_OK;
}

esp_err_t options_handler(httpd_req_t *req)
{
	// Set CORS headers
	httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
	httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "POST, GET, PUT, DELETE, OPTIONS");
	httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type");

	// Respond with a 204 status (No Content)
	httpd_resp_set_status(req, "204");
	httpd_resp_send(req, NULL, 0);

	return ESP_OK;
}

esp_err_t options_restore_handler(httpd_req_t *req)
{
	// Set CORS headers
	httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
	httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "PUT");
	httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type");

	// Respond with a 204 status (No Content)
	httpd_resp_set_status(req, "204");
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
	// cJSON_Delete(item);
	// cJSON_free(item);
}

/**
 * @brief Update layer
 *
 * @param req
 * @return esp_err_t
 */
esp_err_t update_layer_url_handler(httpd_req_t *req)
{

	ESP_LOGI(TAG, "HTTP PUT --> /api/layers");

	ESP_ERROR_CHECK(httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*")); //
	httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type");

	char *string = NULL;
	json_response(string);

	int position = 0;
	// char buffer[1024];
	// httpd_req_recv(req, buffer, req->content_len);
	char *buf;
	size_t buf_len;

	buf_len = (req->content_len) + 1;
	buf = malloc(buf_len);
	httpd_req_recv(req, buf, req->content_len);

	cJSON *payload = cJSON_Parse(buf);
	dd_layer temp_layout;

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

	cJSON *layer_uuid = cJSON_GetObjectItem(payload, "uuid");
	if (cJSON_IsString(layer_uuid) && (layer_uuid->valuestring != NULL))
	{
		printf("Layer uuid = \"%s\"\n", layer_uuid->valuestring);
	}

	int found_flag = 0;
	int pos = 0;
	for (pos = 0; pos < layers_num; pos++)
	{
		if (strcmp(key_layouts[pos].uuid_str, layer_uuid->valuestring) == 0) // If both are equal
		{
			found_flag = 1; // activate found flag
			break;			// Get off the for loop
		}
	}
	if (!found_flag) // IF not found, return http error
	{
		ESP_LOGE(TAG, "UUID not found");
		httpd_resp_set_status(req, HTTPD_400);
		httpd_resp_send(req, NULL, 0);
		return ESP_OK;
	}

	// cJSON *layer_pos = cJSON_GetObjectItem(payload, "pos");
	// if (cJSON_IsNumber(layer_pos) && (layer_pos->valueint))
	// {
	// 	printf("Layer pos = \"%d\"\n", layer_pos->valueint);
	// 	position = layer_pos->valueint;
	// }

	// if (strcmp(key_layouts[position].uuid_str, layer_uuid->valuestring) != 0)
	// {

	// 	ESP_LOGI(TAG, "key layout uuid %s", key_layouts[position].uuid_str);
	// 	ESP_LOGI(TAG, "The string value = %s", layer_uuid->valuestring);

	// 	httpd_resp_set_status(req, HTTPD_400);
	// 	httpd_resp_send(req, NULL, 0);
	// 	return ESP_OK;
	// }
	strcpy(temp_layout.uuid_str, layer_uuid->valuestring);

	cJSON *new_layer_name = cJSON_GetObjectItem(payload, "name");
	if (cJSON_IsString(new_layer_name) && (new_layer_name->valuestring != NULL))
	{
		strcpy(temp_layout.name, new_layer_name->valuestring);
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
	// printf("Names:\n");
	for (i = 0; i < ROWS; i++)
	{
		for (j = 0; j < COLS; j++)
		{
			strcpy(temp_layout.key_map_names[i][j], names[i][j]);
		}
	}
	// printf("\nCodes:\n");
	for (i = 0; i < ROWS; i++)
	{
		for (j = 0; j < COLS; j++)
		{
			// printf("%d\t", codes[i][j]);

			temp_layout.key_map[i][j] = codes[i][j];
		}
		// printf("\n");
	}

	cJSON *item;
	i = 0;
	cJSON *left_encoder_map = cJSON_GetObjectItem(payload, "left_encoder_map");
	if (left_encoder_map == NULL)
	{
		ESP_LOGE(TAG, "left_encoder_map not found");
		return 1;
	}
	cJSON_ArrayForEach(item, left_encoder_map)
	{
		// printf("%s: %d\n", item->string, item->valueint);
		temp_layout.left_encoder_map[i] = item->valueint;
		i++;
	}

	i = 0;
	cJSON *right_encoder_map = cJSON_GetObjectItem(payload, "right_encoder_map");
	cJSON_ArrayForEach(item, right_encoder_map)
	{
		// printf("%s: %d\n", item->string, item->valueint);
		temp_layout.right_encoder_map[i] = item->valueint;
		i++;
	}
	i = 0;
	cJSON *gesture_map = cJSON_GetObjectItem(payload, "gesture_map");
	cJSON_ArrayForEach(item, gesture_map)
	{
		// printf("%s: %d\n", item->string, item->valueint);
		temp_layout.gesture_map[i] = item->valueint;
		i++;
	}

	cJSON *is_active = cJSON_GetObjectItem(payload, "active");
	bool active = cJSON_IsTrue(is_active);
	if (active)
	{
		temp_layout.active = active;
	}
	else
	{
		temp_layout.active = false;
	}
	cJSON_Delete(payload);
	free(buf);
	nvs_write_layer(temp_layout, pos);

	httpd_resp_set_type(req, "application/json");
	httpd_resp_sendstr(req, string);
	httpd_resp_set_status(req, HTTPD_200);
	httpd_resp_send(req, NULL, 0);

	current_layout = 0;
	xQueueSend(layer_recieve_q, &current_layout,
			   (TickType_t)0);

	return ESP_OK;
}

/**
 * @brief Create a new layer
 *
 * @param req
 * @return esp_err_t
 */
esp_err_t create_layer_url_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "HTTP POST  Create Layer --> /api/layers");
	// 	httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
	// httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "*");
	// httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "*");

	ESP_ERROR_CHECK(httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*")); //
	// httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "*");
	httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type");
	// httpd_resp_set_hdr(req, "Access-Control-Allow-Credentials", "true");
	// httpd_resp_set_hdr(req, "Access-Control-Max-Age", "3600");
	// httpd_resp_set_hdr(req, "Access-Control-Expose-Headers", "X-Custom-Header");
	// httpd_resp_set_hdr(req, "Vary", "Origin");
	// char buffer[1024];
	char *buf;
	size_t buf_len;
	char *string = NULL;
	json_response(string);

	buf_len = (req->content_len) + 1;
	buf = malloc(buf_len);
	httpd_req_recv(req, buf, req->content_len);

	dd_layer new_layer;
	esp_err_t res;
	cJSON *payload = cJSON_Parse(buf);

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

	cJSON *layer_name = cJSON_GetObjectItem(payload, "name");
	if (cJSON_IsString(layer_name) && (layer_name->valuestring != NULL))
	{
		// printf("Layer Name = \"%s\"\n", layer_name->valuestring);

		strcpy(new_layer.name, layer_name->valuestring);
		// printf("ddLayer Name = \"%s\"\n", new_layer.name);
	}

	cJSON *layer_uuid = cJSON_GetObjectItem(payload, "uuid");
	if (cJSON_IsString(layer_uuid) && (layer_uuid->valuestring != NULL))
	{
		// printf("Layer uuid = \"%s\"\n", layer_uuid->valuestring);
		strcpy(new_layer.uuid_str, layer_uuid->valuestring);
		// printf("ddLayer uuid = \"%s\"\n", new_layer.uuid_str);
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

	ESP_LOGI(TAG, "HTTP POST  Create Layer --> /api/layers - Working so far?");

	fill_row(row0, names[0], codes[0]);
	fill_row(row1, names[1], codes[1]);
	fill_row(row2, names[2], codes[2]);
	fill_row(row3, names[3], codes[3]);

	ESP_LOGI(TAG, "HTTP POST  Create Layer --> /api/layers - Working so far2?");

	int i, j;
	// printf("Names:\n");
	for (i = 0; i < ROWS; i++)
	{
		for (j = 0; j < COLS; j++)
		{
			// printf("%s\t", names[i][j]);
			// strcpy(key_layouts[edit_layer]->key_map_names[i][j], names[i][j]);
			strcpy(new_layer.key_map_names[i][j], names[i][j]);
		}
		// printf("\n");
	}

	// printf("\nCodes:\n");
	for (i = 0; i < ROWS; i++)
	{
		for (j = 0; j < COLS; j++)
		{
			// printf("%d\t", codes[i][j]);
			// key_layouts[edit_layer]->key_map[i][j] = codes[i][j];
			new_layer.key_map[i][j] = codes[i][j];
		}
		printf("\n");
	}
	cJSON_Delete(payload);
	free(buf);
	current_layout = 0;
	res = nvs_create_new_layer(new_layer);
	if (res == ESP_OK)
	{
		httpd_resp_set_type(req, "application/json");
		httpd_resp_sendstr(req, string);
		httpd_resp_set_status(req, HTTPD_200);
		httpd_resp_send(req, NULL, 0);
		xQueueSend(layer_recieve_q, &current_layout,
				   (TickType_t)0);
	}
	else
	{
		// TODO: Handle error -> maximum number of layers reached
		xQueueSend(layer_recieve_q, &current_layout,
				   (TickType_t)0);
		httpd_resp_set_status(req, HTTPD_400);
		httpd_resp_send(req, NULL, 0);
	}

	return ESP_OK;
}

/**
 * @brief Restore Default Layouts
 *
 * @param req
 * @return esp_err_t
 */
esp_err_t restore_default_layer_url_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "HTTP POST  Restore Default Layouts --> /api/layers/restore");

	ESP_ERROR_CHECK(httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*")); //
	// httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "*");
	httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type");
	// httpd_resp_set_hdr(req, "Access-Control-Allow-Credentials", "true");
	// httpd_resp_set_hdr(req, "Access-Control-Max-Age", "3600");
	// httpd_resp_set_hdr(req, "Access-Control-Expose-Headers", "X-Custom-Header");
	// httpd_resp_set_hdr(req, "Vary", "Origin");

	char *string = NULL;
	json_response(string);
	esp_err_t error = nvs_restore_default_layers();
	if (error == ESP_OK)
	{

		httpd_resp_set_type(req, "application/json");
		httpd_resp_sendstr(req, string);
		httpd_resp_set_status(req, HTTPD_200);
		httpd_resp_send(req, NULL, 0);
	}
	else
	{

		httpd_resp_set_status(req, HTTPD_400);
		httpd_resp_send(req, NULL, 0);
	}
	current_layout = 0;
	xQueueSend(layer_recieve_q, &current_layout,
			   (TickType_t)0);

	return ESP_OK;
}

/**
 * @brief Change Keyboard LED Color
 *
 * @param req
 * @return esp_err_t
 */
esp_err_t change_keyboard_led_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "HTTP POST  CHANGE LED MODE --> /api/led");
	int mode_t;
	// httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
	// httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "*");
	// httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "*");

	httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
	// httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "*");
	httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type");
	// httpd_resp_set_hdr(req, "Access-Control-Allow-Credentials", "true");
	// httpd_resp_set_hdr(req, "Access-Control-Max-Age", "3600");
	// httpd_resp_set_hdr(req, "Access-Control-Expose-Headers", "X-Custom-Header");
	// httpd_resp_set_hdr(req, "Vary", "Origin");

	// Read the URI line and get the host
	char *buf;
	size_t buf_len;
	char int_param[3];
	char *string = NULL;

	// cJSON *monitor = cJSON_CreateObject();
	// if (monitor == NULL)
	// {
	// 	httpd_resp_set_status(req, HTTPD_400);
	// 	httpd_resp_send(req, NULL, 0);
	// 	return ESP_OK;
	// }

	// cJSON *_name = cJSON_CreateString("success");
	// cJSON_AddItemToObject(monitor, "name", _name);
	// string = cJSON_Print(monitor);

	json_response(string);

	// Read the URI line and get the parameters
	buf_len = httpd_req_get_url_query_len(req) + 1;
	if (buf_len > 1)
	{
		buf = malloc(buf_len);
		if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK)
		{
			ESP_LOGI(TAG, "Found URL query: %s", buf);

			if (httpd_query_key_value(buf, "mode", int_param, sizeof(int_param)) == ESP_OK)
			{
				ESP_LOGI(TAG, "Led Mode is = %s", int_param);
				mode_t = atoi(int_param);
				xQueueSend(keyled_q, &mode_t, 0);

				httpd_resp_set_type(req, "application/json");
				httpd_resp_sendstr(req, string);
				httpd_resp_set_status(req, HTTPD_200);
				httpd_resp_send(req, NULL, 0);
			}
			else
			{

				httpd_resp_set_status(req, HTTPD_400);
				httpd_resp_send(req, NULL, 0);
			}
		}
		free(buf);
	}
	// cJSON_Delete(monitor);
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

/////////////////////////////////////////////////////////////////////////////////////////////
/* Set HTTP response content type according to file extension */
static esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filepath)
{
	const char *type = "text/plain";
	if (CHECK_FILE_EXTENSION(filepath, ".html"))
	{
		type = "text/html";
	}
	else if (CHECK_FILE_EXTENSION(filepath, ".js"))
	{
		type = "application/javascript";
	}
	else if (CHECK_FILE_EXTENSION(filepath, ".css"))
	{
		type = "text/css";
	}
	else if (CHECK_FILE_EXTENSION(filepath, ".png"))
	{
		type = "image/png";
	}
	else if (CHECK_FILE_EXTENSION(filepath, ".ico"))
	{
		type = "image/x-icon";
	}
	else if (CHECK_FILE_EXTENSION(filepath, ".svg"))
	{
		type = "text/xml";
	}
	else if (CHECK_FILE_EXTENSION(filepath, ".gzip"))
	{
		type = "application/x-gzip";
	}

	return httpd_resp_set_type(req, type);
}

/* Send HTTP response with the contents of the requested file */
static esp_err_t rest_common_get_handler(httpd_req_t *req)
{

	char filepath[FILE_PATH_MAX];

	rest_server_context_t *rest_context = (rest_server_context_t *)req->user_ctx;
	strlcpy(filepath, rest_context->base_path, sizeof(filepath));

	ESP_LOGI(REST_TAG, "URI %s", req->uri);

	if (req->uri[strlen(req->uri) - 1] == '/')
	{
		strlcat(filepath, "/index.html", sizeof(filepath));
	}
	else
	{
		strlcat(filepath, req->uri, sizeof(filepath));
	}

	set_content_type_from_file(req, filepath);

	int fd = open(filepath, O_RDONLY, 0);
	if (fd == -1)
	{
		strlcat(filepath, ".gz", sizeof(filepath));
		fd = open(filepath, O_RDONLY, 0);
		if (fd == -1)
		{
			ESP_LOGE(REST_TAG, "Failed to open file : %s", filepath);
			/* Respond with 500 Internal Server Error */
			httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");
			return ESP_FAIL;
		}
		httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
	}

	char *chunk = rest_context->scratch;
	ssize_t read_bytes;
	do
	{
		/* Read file in chunks into the scratch buffer */
		read_bytes = read(fd, chunk, SCRATCH_BUFSIZE);
		if (read_bytes == -1)
		{
			ESP_LOGE(REST_TAG, "Failed to read file : %s", filepath);
		}
		else if (read_bytes > 0)
		{
			/* Send the buffer contents as HTTP response chunk */
			if (httpd_resp_send_chunk(req, chunk, read_bytes) != ESP_OK)
			{
				close(fd);
				ESP_LOGE(REST_TAG, "File sending failed!");
				/* Abort sending file */
				httpd_resp_sendstr_chunk(req, NULL);
				/* Respond with 500 Internal Server Error */
				httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
				return ESP_FAIL;
			}
		}
	} while (read_bytes > 0);
	/* Close file after sending complete */
	close(fd);
	ESP_LOGI(REST_TAG, "File sending complete");
	/* Respond with an empty chunk to signal HTTP response completion */
	httpd_resp_send_chunk(req, NULL, 0);
	return ESP_OK;
}

/*
 *OTA
 */

esp_err_t get_handler(httpd_req_t *req)
{
	/* Send a simple response */
	httpd_resp_send(req, (const char *)index_html_start, index_html_end - index_html_start);
	return ESP_OK;
}

esp_err_t post_handler(httpd_req_t *req)
{

	char buf[1000];
	esp_ota_handle_t ota_handle;
	int remaining = req->content_len;

	const esp_partition_t *ota_partition = esp_ota_get_next_update_partition(NULL);
	ESP_ERROR_CHECK(esp_ota_begin(ota_partition, OTA_SIZE_UNKNOWN, &ota_handle));

	while (remaining > 0)
	{
		int recv_len = httpd_req_recv(req, buf, MIN(remaining, sizeof(buf)));

		// Timeout Error: Just retry
		if (recv_len == HTTPD_SOCK_ERR_TIMEOUT)
		{
			continue;

			// Serious Error: Abort OTA
		}
		else if (recv_len <= 0)
		{
			httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Protocol Error");
			return ESP_FAIL;
		}

		// Successful Upload: Flash firmware chunk
		if (esp_ota_write(ota_handle, (const void *)buf, recv_len) != ESP_OK)
		{
			httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Flash Error");
			return ESP_FAIL;
		}

		remaining -= recv_len;
	}

	// Validate and switch to new OTA image and reboot
	if (esp_ota_end(ota_handle) != ESP_OK || esp_ota_set_boot_partition(ota_partition) != ESP_OK)
	{
		httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Validation / Activation Error");
		return ESP_FAIL;
	}

	httpd_resp_sendstr(req, "Firmware update complete, rebooting now!\n");

	vTaskDelay(500 / portTICK_PERIOD_MS);
	esp_restart();

	return ESP_OK;
}

/**
 * @brief
 *
 * @return httpd_handle_t
 */
httpd_handle_t start_webserver(const char *base_path)
{
	REST_CHECK(base_path, "wrong base path", err);
	rest_server_context_t *rest_context = calloc(1, sizeof(rest_server_context_t));
	REST_CHECK(rest_context, "No memory for rest context", err);
	strlcpy(rest_context->base_path, base_path, sizeof(rest_context->base_path));

	httpd_handle_t server = NULL;
	httpd_config_t config = HTTPD_DEFAULT_CONFIG();
	config.max_uri_handlers = 20;
	config.stack_size = 1024 * 10;
	config.uri_match_fn = httpd_uri_match_wildcard;

	// Start the httpd server
	// ESP_ERROR_CHECK(httpd_start(&server, &config));
	REST_CHECK(httpd_start(&server, &config) == ESP_OK, "Start server failed", err_start);

	ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);

	httpd_uri_t connect_url = {.uri = "/api/connect", .method = HTTP_POST, .handler = connect_url_handler, .user_ctx = NULL};
	httpd_register_uri_handler(server, &connect_url);

	httpd_uri_t get_config_url = {.uri = "/api/config", .method = HTTP_GET, .handler = config_url_handler, .user_ctx = NULL};
	httpd_register_uri_handler(server, &get_config_url);

	////////LED
	httpd_uri_t change_led_color_url = {.uri = "/api/led", .method = HTTP_POST, .handler = change_keyboard_led_handler, .user_ctx = NULL};
	httpd_register_uri_handler(server, &change_led_color_url);
	httpd_uri_t change_led_color_url__ = {.uri = "/api/led", .method = HTTP_OPTIONS, .handler = change_keyboard_led_handler, .user_ctx = NULL};
	httpd_register_uri_handler(server, &change_led_color_url__);

	///////LAYERS

	httpd_uri_t get_layer_url = {.uri = "/api/layers", .method = HTTP_GET, .handler = get_layer_url_handler, .user_ctx = NULL};
	httpd_register_uri_handler(server, &get_layer_url);
	httpd_uri_t get_layerName_url = {.uri = "/api/layers/layer_names", .method = HTTP_GET, .handler = get_layerName_url_handler, .user_ctx = NULL};
	httpd_register_uri_handler(server, &get_layerName_url);
	httpd_uri_t delete_layer_url = {.uri = "/api/layers", .method = HTTP_DELETE, .handler = delete_layer_url_handler, .user_ctx = NULL};
	httpd_register_uri_handler(server, &delete_layer_url);
	httpd_uri_t create_layer_url = {.uri = "/api/layers", .method = HTTP_POST, .handler = create_layer_url_handler, .user_ctx = NULL};
	httpd_register_uri_handler(server, &create_layer_url);
	httpd_uri_t layer_options_url = {.uri = "/api/layers", .method = HTTP_OPTIONS, .handler = options_handler, .user_ctx = NULL};
	httpd_register_uri_handler(server, &layer_options_url);

	httpd_uri_t update_layer_url = {.uri = "/api/layers", .method = HTTP_PUT, .handler = update_layer_url_handler, .user_ctx = NULL};
	httpd_register_uri_handler(server, &update_layer_url);

	httpd_uri_t restore_default_layer_url = {.uri = "/api/layers/restore", .method = HTTP_PUT, .handler = restore_default_layer_url_handler, .user_ctx = NULL};
	httpd_register_uri_handler(server, &restore_default_layer_url);
	httpd_uri_t layer_restore_options_url = {.uri = "/api/layers/restore", .method = HTTP_OPTIONS, .handler = options_restore_handler, .user_ctx = NULL};
	httpd_register_uri_handler(server, &layer_restore_options_url);

	// MACROS
	httpd_uri_t get_macros_url = {.uri = "/api/macros", .method = HTTP_GET, .handler = get_macros_url_handler, .user_ctx = NULL};
	httpd_register_uri_handler(server, &get_macros_url);
	httpd_uri_t create_macro_url = {.uri = "/api/macros", .method = HTTP_POST, .handler = create_macro_url_handler, .user_ctx = NULL};
	httpd_register_uri_handler(server, &create_macro_url);
	httpd_uri_t delete_macro_url = {.uri = "/api/macros", .method = HTTP_DELETE, .handler = delete_macro_url_handler, .user_ctx = NULL};
	httpd_register_uri_handler(server, &delete_macro_url);
	httpd_uri_t update_macro_url = {.uri = "/api/macros", .method = HTTP_PUT, .handler = update_macro_url_handler, .user_ctx = NULL};
	httpd_register_uri_handler(server, &update_macro_url);
	httpd_uri_t option_macros_url = {.uri = "/api/macros", .method = HTTP_OPTIONS, .handler = options_handler, .user_ctx = NULL};
	httpd_register_uri_handler(server, &option_macros_url);
	httpd_uri_t restore_all_macro_url = {.uri = "/api/macros/restore", .method = HTTP_POST, .handler = restore_default_macro_url_handler, .user_ctx = NULL};
	httpd_register_uri_handler(server, &restore_all_macro_url);

	// OTA URI
	/* URI handler structure for GET /uri */
	httpd_uri_t uri_get = {
		.uri = "/",
		.method = HTTP_GET,
		.handler = get_handler,
		.user_ctx = NULL};
	httpd_register_uri_handler(server, &uri_get);

	/* URI handler structure for POST /uri */
	httpd_uri_t uri_post = {
		.uri = "/update",
		.method = HTTP_POST,
		.handler = post_handler,
		.user_ctx = NULL};

	httpd_register_uri_handler(server, &uri_post);
	//////

	/* URI handler for getting web server files */
	httpd_uri_t common_get_uri = {
		.uri = "/test/*",
		//.uri = "/esp-portal/*",
		.method = HTTP_GET,
		.handler = rest_common_get_handler,
		.user_ctx = rest_context};
	httpd_register_uri_handler(server, &common_get_uri);

	return server;
err_start:
	free(rest_context);
err:
	return NULL;
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
