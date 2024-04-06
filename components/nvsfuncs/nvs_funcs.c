/**
 * @file nvs_funcs.c
 * @author ElectroNick (nick@dsd.dev)
 * @brief
 * @version 0.1
 * @date 2022-12-10
 *
 * @copyright Copyright (c) 2022
 * Based on the code f Gal Zaidenstein.
 *
 * MIT License
 * Copyright (c) 2022
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * DeepDeck, a product by DeepSea Developments.
 * More info on DeepDeck @ www.deepdeck.co
 * DeepseaDev.com
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "nvs_funcs.h"
#include "keymap.h"
#include "key_definitions.h"
#include "keymap.h"

#define NVS_TAG "NVS Storage"

const static char *TAG = "NVS FUNCS";

// TODO: should not have global variables, but encapsulated ones.

/**
 * @brief Check the number of available entries of the NVS
 *
 */
void nvs_check_memory_status(void)
{
	nvs_stats_t nvs_stats;
	nvs_get_stats(NULL, &nvs_stats);
	ESP_LOGI(TAG, "Free memory: %d bytes", esp_get_free_heap_size());
	ESP_LOGI(TAG, "Count: UsedEntries = (%zu), FreeEntries = (%zu), AllEntries = (%zu), Namespace_count = (%zu)",
			 nvs_stats.used_entries, nvs_stats.free_entries, nvs_stats.total_entries, nvs_stats.namespace_count);
}

/**********
 * NVS COMMON
 ***********/

typedef struct
{
	char name[NVS_NS_NAME_MAX_SIZE];
} lst_item_t;

esp_err_t nvs_list_init(const char *list_name)
{

	nvs_handle_t nvs_handle;

	// Open namespace
	ESP_ERROR_CHECK(nvs_open(list_name,
							 NVS_READWRITE,
							 &nvs_handle));

	// Clean namespace
	nvs_erase_all(nvs_handle);

	char lst_namespace[NVS_NS_NAME_MAX_SIZE];

	sprintf(lst_namespace, "%s_ls", list_name);

	// Init Empty List
	ESP_ERROR_CHECK(nvs_set_blob(nvs_handle,
								 lst_namespace,
								 NULL,
								 0));

	ESP_ERROR_CHECK(nvs_commit(nvs_handle));

	nvs_close(nvs_handle);

	return ESP_OK;
}
esp_err_t nvs_list_initialized(const char *list_name)
{
	nvs_handle_t nvs_handle;

	// ESP_LOGW(TAG, "%s", __func__);

	// Open namespace
	ESP_ERROR_CHECK(nvs_open(list_name,
							 NVS_READWRITE,
							 &nvs_handle));

	char lst_namespace[NVS_NS_NAME_MAX_SIZE];

	sprintf(lst_namespace, "%s_ls", list_name);

	// Read list to get size
	size_t list_size;
	esp_err_t error = nvs_get_blob(nvs_handle,
								   (const char *)lst_namespace,
								   NULL,
								   &list_size);

	nvs_close(nvs_handle);

	ESP_ERROR_CHECK_WITHOUT_ABORT(error);

	return error;
}

esp_err_t nvs_list_add_item(const char *list_name, void *item, size_t item_size)
{
	nvs_handle_t nvs_handle;

	// Open namespace
	ESP_ERROR_CHECK(nvs_open(list_name,
							 NVS_READWRITE,
							 &nvs_handle));

	char lst_namespace[NVS_NS_NAME_MAX_SIZE];

	sprintf(lst_namespace, "%s_ls", list_name);

	// Read list to get size
	size_t list_size;
	esp_err_t ret = nvs_get_blob(nvs_handle,
								 (const char *)lst_namespace,
								 NULL,
								 &list_size);

	if (ret == ESP_ERR_NVS_NOT_FOUND)
	{
		list_size = 0;
	}
	else
	{
		ESP_ERROR_CHECK(ret);
	}

	size_t new_size = list_size + sizeof(lst_item_t);

	lst_item_t *lst_item = (lst_item_t *)malloc(new_size);

	// Read list to get data

	ESP_ERROR_CHECK(nvs_get_blob(nvs_handle,
								 (const char *)lst_namespace,
								 (void *)lst_item,
								 &list_size));

	size_t item_index = list_size / sizeof(lst_item_t);

	// Generate short id
	uuid_t uu;
	uuid_generate(uu);
	short_uuid_unparse(uu, lst_item[item_index].name);

	// Store new item content
	ESP_ERROR_CHECK(nvs_set_blob(nvs_handle,
								 (const char *)lst_item[item_index].name,
								 (void *)item,
								 item_size));

	//ESP_LOG_BUFFER_HEXDUMP(TAG, item, item_size, ESP_LOG_WARN);

	// ESP_LOGW(TAG, "add item name[%u] : %s", item_index, lst_item[item_index].name);

	// Save updated list
	ESP_ERROR_CHECK(nvs_set_blob(nvs_handle,
								 lst_namespace,
								 (void *)lst_item,
								 new_size));

	ESP_ERROR_CHECK(nvs_commit(nvs_handle));

	nvs_close(nvs_handle);

	free(lst_item);
	return ESP_OK;
}
esp_err_t nvs_list_default(const char *list_name, void *list, size_t list_len, size_t item_size)
{
	nvs_list_init(list_name);

	uint32_t address = (uint32_t)list;

	for (size_t i = 0; i < list_len; i++)
	{
		nvs_list_add_item(list_name, (void *)address, item_size);
		address += item_size;
	}
	return ESP_OK;
}
esp_err_t nvs_list_auto_default(const char *list_name, void *list, size_t list_len, size_t item_size)
{
	esp_err_t ret = nvs_list_initialized(list_name);

	if (ret != ESP_OK)
	{
		nvs_list_default(list_name, list, list_len, item_size);
	}
	return ESP_OK;
}
esp_err_t nvs_list_load(const char *list_name, void **list, size_t *list_len, size_t item_size)
{

	nvs_handle_t nvs_handle;

	// Open namespace
	ESP_ERROR_CHECK(nvs_open(list_name,
							 NVS_READWRITE,
							 &nvs_handle));

	char lst_namespace[NVS_NS_NAME_MAX_SIZE];

	sprintf(lst_namespace, "%s_ls", list_name);

	// Read list to get size
	size_t list_size;
	esp_err_t ret = nvs_get_blob(nvs_handle,
								 (const char *)lst_namespace,
								 NULL,
								 &list_size);

	if (ret == ESP_ERR_NVS_NOT_FOUND)
	{
		list_size = 0;
	}
	else
	{
		ESP_ERROR_CHECK(ret);
	}

	lst_item_t *lst_item = (lst_item_t *)malloc(list_size);

	// Read list to get data
	ESP_ERROR_CHECK(nvs_get_blob(nvs_handle,
								 (const char *)lst_namespace,
								 (void *)lst_item,
								 &list_size));

	list_size = list_size / sizeof(lst_item_t);

	// allocate or reallocate space for load list data
	if (*list != NULL)
	{
		if (list_size == 0)
		{
			free(*list);
		}
		else
		{
			*list = realloc(*list, item_size * list_size);
		}
	}
	else
	{
		*list = malloc(item_size * list_size);
	}

	uint32_t address = (uint32_t)*list;

	for (size_t i = 0; i < list_size; i++)
	{

		// ESP_LOGW(TAG, "read item name[%u] : %s", i, lst_item[i].name);

		size_t desired_item_size = item_size;
		// Read list to get item data
		ESP_ERROR_CHECK(nvs_get_blob(nvs_handle,
									 (const char *)lst_item[i].name,
									 (void *)address,
									 &desired_item_size));
		// ESP_LOG_BUFFER_HEXDUMP(TAG, address, desired_item_size, ESP_LOG_WARN);

		address += item_size;
	}

	*list_len = list_size;

	nvs_close(nvs_handle);

	return ESP_OK;
}

esp_err_t nvs_list_update(const char *list_name, size_t index, void *item, size_t item_size)
{
	nvs_handle_t nvs_handle;

	// Open namespace
	ESP_ERROR_CHECK(nvs_open(list_name,
							 NVS_READWRITE,
							 &nvs_handle));

	char lst_namespace[NVS_NS_NAME_MAX_SIZE];

	sprintf(lst_namespace, "%s_ls", list_name);

	// ESP_LOGW(TAG, "lst_namespace %s", lst_namespace);

	// Read list to get size
	size_t list_size;
	esp_err_t ret = nvs_get_blob(nvs_handle,
								 (const char *)lst_namespace,
								 NULL,
								 &list_size);

	if (ret == ESP_ERR_NVS_NOT_FOUND)
	{
		list_size = 0;
	}
	else
	{
		ESP_ERROR_CHECK(ret);
	}

	lst_item_t *lst_item = (lst_item_t *)malloc(list_size);

	// Read list to get data

	ESP_ERROR_CHECK(nvs_get_blob(nvs_handle,
								 (const char *)lst_namespace,
								 (void *)lst_item,
								 &list_size));

	size_t list_len = list_size / sizeof(lst_item_t);

	if (index < list_len)
	{

		// ESP_LOGW(TAG, "update item name[%u] : %s", index, lst_item[index].name);

		// Store new item content
		ESP_ERROR_CHECK(nvs_set_blob(nvs_handle,
									 (const char *)lst_item[index].name,
									 (void *)item,
									 item_size));

		ESP_ERROR_CHECK(nvs_commit(nvs_handle));
	}

	nvs_close(nvs_handle);

	free(lst_item);
	return ESP_OK;
}

esp_err_t nvs_list_delete_item(const char *list_name, size_t index)
{
	nvs_handle_t nvs_handle;

	// Open namespace
	ESP_ERROR_CHECK(nvs_open(list_name,
							 NVS_READWRITE,
							 &nvs_handle));

	char lst_namespace[NVS_NS_NAME_MAX_SIZE];

	sprintf(lst_namespace, "%s_ls", list_name);

	// Read list to get size
	size_t list_size;
	esp_err_t ret = nvs_get_blob(nvs_handle,
								 (const char *)lst_namespace,
								 NULL,
								 &list_size);

	if (ret == ESP_ERR_NVS_NOT_FOUND)
	{
		list_size = 0;
	}
	else
	{
		ESP_ERROR_CHECK(ret);
	}

	size_t list_len = list_size / sizeof(lst_item_t);

	lst_item_t *lst_item = (lst_item_t *)malloc(list_size);

	if (index < list_len)
	{
		list_len--;
		size_t new_size = list_size - sizeof(lst_item_t);

		// Read list to get data
		ESP_ERROR_CHECK(nvs_get_blob(nvs_handle,
									 (const char *)lst_namespace,
									 (void *)lst_item,
									 &list_size));

		// Erase item content
		ESP_ERROR_CHECK(nvs_erase_key(nvs_handle,
									  lst_item[index].name));

		// Remove item in list and reorganize it.
		for (int i = index; i < list_len; i++)
		{
			strcpy(lst_item[i].name, lst_item[i + 1].name);
		}

		// Save updated list
		ESP_ERROR_CHECK(nvs_set_blob(nvs_handle,
									 lst_namespace,
									 (void *)lst_item,
									 new_size));

		ESP_ERROR_CHECK(nvs_commit(nvs_handle));
	}

	nvs_close(nvs_handle);

	free(lst_item);
	return ESP_OK;
}

esp_err_t nvs_list_organize(const char *list_name, size_t item_size, selector_t selector_cb)
{
	nvs_handle_t nvs_handle;

	if (!selector_cb)
		return ESP_FAIL;

	// Open namespace
	ESP_ERROR_CHECK(nvs_open(list_name,
							 NVS_READWRITE,
							 &nvs_handle));

	char lst_namespace[NVS_NS_NAME_MAX_SIZE];

	sprintf(lst_namespace, "%s_ls", list_name);

	// Read list to get size
	size_t list_size;
	esp_err_t ret = nvs_get_blob(nvs_handle,
								 (const char *)lst_namespace,
								 NULL,
								 &list_size);

	if (ret == ESP_ERR_NVS_NOT_FOUND)
	{
		list_size = 0;
	}
	else
	{
		ESP_ERROR_CHECK(ret);
	}

	lst_item_t *lst_item = (lst_item_t *)malloc(list_size);
	lst_item_t *lst_item_new = (lst_item_t *)malloc(list_size);

	// Read list to get data

	ESP_ERROR_CHECK(nvs_get_blob(nvs_handle,
								 (const char *)lst_namespace,
								 (void *)lst_item,
								 &list_size));

	size_t list_len = list_size / sizeof(lst_item_t);

	void *item = malloc(item_size);

	uint8_t aux_counter = 0;
	for (size_t i = 0; i < list_len; i++)
	{

		size_t desired_item_size = item_size;
		// Read list to get item data
		ESP_ERROR_CHECK(nvs_get_blob(nvs_handle,
									 (const char *)lst_item[i].name,
									 (void *)item,
									 &desired_item_size));

		if (selector_cb(item))
		{
			// ESP_LOGW(TAG, "selected item name[%u] : %s", i, lst_item[i].name);
			strcpy(lst_item_new[aux_counter].name, lst_item[i].name);
			aux_counter++;
		}
	}
	for (size_t i = 0; i < list_len; i++)
	{

		// ESP_LOGW(TAG, "not selected item name[%u] : %s", i, lst_item[i].name);

		size_t desired_item_size = item_size;
		// Read list to get item data
		ESP_ERROR_CHECK(nvs_get_blob(nvs_handle,
									 (const char *)lst_item[i].name,
									 (void *)item,
									 &desired_item_size));

		if (!selector_cb(item))
		{
			strcpy(lst_item_new[aux_counter].name, lst_item[i].name);
			aux_counter++;
		}
	}

	// Save updated list
	ESP_ERROR_CHECK(nvs_set_blob(nvs_handle,
								 lst_namespace,
								 (void *)lst_item_new,
								 list_size));

	ESP_ERROR_CHECK(nvs_commit(nvs_handle));

	nvs_close(nvs_handle);

	free(lst_item);
	free(lst_item_new);

	return ESP_OK;
}
esp_err_t nvs_list_get_len(const char *list_name, size_t *lst_len)
{
	nvs_handle_t nvs_handle;

	// Open namespace
	ESP_ERROR_CHECK(nvs_open(list_name,
							 NVS_READWRITE,
							 &nvs_handle));

	char lst_namespace[NVS_NS_NAME_MAX_SIZE];

	sprintf(lst_namespace, "%s_ls", list_name);

	// Read list to get size
	size_t list_size;
	esp_err_t ret = nvs_get_blob(nvs_handle,
								 (const char *)lst_namespace,
								 NULL,
								 &list_size);

	if (ret == ESP_ERR_NVS_NOT_FOUND)
	{
		list_size = 0;
	}
	else
	{
		ESP_ERROR_CHECK(ret);
	}

	lst_item_t *lst_item = (lst_item_t *)malloc(list_size);

	// Read list to get data
	ESP_ERROR_CHECK(nvs_get_blob(nvs_handle,
								 (const char *)lst_namespace,
								 (void *)lst_item,
								 &list_size));

	*lst_len = list_size / sizeof(lst_item_t);

	// ESP_LOGW(TAG, "list len : %u", *lst_len);

	free(lst_item);

	nvs_close(nvs_handle);

	return ESP_OK;
}
esp_err_t nvs_list_load_item(const char *list_name, size_t index, void **item, size_t item_size)
{

	nvs_handle_t nvs_handle;

	// Open namespace
	ESP_ERROR_CHECK(nvs_open(list_name,
							 NVS_READWRITE,
							 &nvs_handle));

	char lst_namespace[NVS_NS_NAME_MAX_SIZE];

	sprintf(lst_namespace, "%s_ls", list_name);

	// Read list to get size
	size_t list_size;
	esp_err_t ret = nvs_get_blob(nvs_handle,
								 (const char *)lst_namespace,
								 NULL,
								 &list_size);

	if (ret == ESP_ERR_NVS_NOT_FOUND)
	{
		list_size = 0;
	}
	else
	{
		ESP_ERROR_CHECK(ret);
	}

	lst_item_t *lst_item = (lst_item_t *)malloc(list_size);

	// Read list to get data
	ESP_ERROR_CHECK(nvs_get_blob(nvs_handle,
								 (const char *)lst_namespace,
								 (void *)lst_item,
								 &list_size));

	list_size = list_size / sizeof(lst_item_t);

	if (index < list_size)
	{

		// allocate or reallocate space for load list data
		if (*item != NULL)
		{
			if (list_size == 0)
			{
				free(*item);
			}
			else
			{
				*item = realloc(*item, item_size);
				// ESP_LOGW(TAG, "realloc");
			}
		}
		else
		{
			*item = malloc(item_size);
			// ESP_LOGW(TAG, "malloc");
		}

		// ESP_LOGW(TAG, "read item name[%u] : %s", index, lst_item[index].name);

		size_t desired_item_size = item_size;
		// Read list to get item data
		ESP_ERROR_CHECK(nvs_get_blob(nvs_handle,
									 (const char *)lst_item[index].name,
									 (void *)*item,
									 &desired_item_size));
		//ESP_LOG_BUFFER_HEXDUMP(TAG, *item, desired_item_size, ESP_LOG_WARN);
	}

	free(lst_item);
	nvs_close(nvs_handle);

	return ESP_OK;
}
// dynamic size items
esp_err_t nvs_item_load(const char *namespace, const char *item_name, void **item, size_t *item_size)
{

	nvs_handle_t nvs_handle;

	// ESP_LOGW(TAG, "read sub item name : %s", item_name);

	// Open namespace
	ESP_ERROR_CHECK(nvs_open(namespace,
							 NVS_READWRITE,
							 &nvs_handle));

	// Read item to get size
	size_t item_size_nvs;
	esp_err_t ret = nvs_get_blob(nvs_handle,
								 (const char *)item_name,
								 NULL,
								 &item_size_nvs);

	if (ret == ESP_ERR_NVS_NOT_FOUND)
	{
		item_size_nvs = 0;
	}
	else
	{
		ESP_ERROR_CHECK(ret);
	}

	// allocate or reallocate space for load item data
	if (*item != NULL)
	{
		if (item_size_nvs == 0)
		{
			free(*item);
		}
		else
		{
			*item = realloc(*item, item_size_nvs);
		}
	}
	else
	{
		*item = malloc(item_size_nvs);
	}

	size_t desired_item_size = item_size_nvs;
	ESP_ERROR_CHECK(nvs_get_blob(nvs_handle,
								 (const char *)item_name,
								 (void *)*item,
								 &desired_item_size));

	*item_size = item_size_nvs;

	nvs_close(nvs_handle);

	return ESP_OK;
}

esp_err_t nvs_item_store(const char *namespace, const char *item_name, void *item, size_t item_size)
{
	nvs_handle_t nvs_handle;

	// ESP_LOGW(TAG, "write sub item name : %s", item_name);

	// Open namespace
	ESP_ERROR_CHECK(nvs_open(namespace,
							 NVS_READWRITE,
							 &nvs_handle));

	// Store new item content
	ESP_ERROR_CHECK(nvs_set_blob(nvs_handle,
								 (const char *)item_name,
								 (void *)item,
								 item_size));

	ESP_ERROR_CHECK(nvs_commit(nvs_handle));

	nvs_close(nvs_handle);

	return ESP_OK;
}

esp_err_t nvs_delete_item(const char *namespace, const char *item_name)
{
	nvs_handle_t nvs_handle;

	// ESP_LOGW(TAG, "delete sub item name : %s", item_name);

	// Open namespace
	ESP_ERROR_CHECK(nvs_open(namespace,
							 NVS_READWRITE,
							 &nvs_handle));

	// Erase item content
	ESP_ERROR_CHECK(nvs_erase_key(nvs_handle,
								  item_name));

	ESP_ERROR_CHECK(nvs_commit(nvs_handle));

	nvs_close(nvs_handle);

	return ESP_OK;
}
