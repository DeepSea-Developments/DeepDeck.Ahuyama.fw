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


// Layers
#define LAYER_NAMESPACE "layers"
#define LAYER_NUM_KEY 	"layer_num"
#define LAYER_LIST_KEY 	"layer_list"


// NameSpaces
#define MACROS_NAMESPACE "user_macros"
#define TAPDANCE_NAMESPACE "user_tapd"
#define LEDMODE_NAMESPACE "led_mode"

// Keys
#define MACROS_KEY "macros_key"
#define TAPDANCE_NUM_KEY "tp_num"

const static char *TAG = "NVS FUNCS";

dd_layer *key_layouts;
dd_macros *user_macros;
dd_tapdance *user_tapdance;


uint8_t layers_num = 0;
uint8_t total_macros = 0;
uint8_t total_tapdance = 0;


/**
 * @brief Check the number of available entries of the NVS
 *
 */
void nvs_check_memory_status(void)
{
	nvs_stats_t nvs_stats;
	nvs_get_stats(NULL, &nvs_stats);
	ESP_LOGI("NVS Status", "Count: UsedEntries = (%zu), FreeEntries = (%zu), AllEntries = (%zu), Namespace_count = (%zu)",
			 nvs_stats.used_entries, nvs_stats.free_entries, nvs_stats.total_entries, nvs_stats.namespace_count);
}

/**
 * @brief Read and returns the number of available layers stored
 *
 * @return uint8_t
 */
uint8_t nvs_read_num_layers(void)
{
	ESP_LOGI(TAG, "READ NUM LAYERS");
	nvs_handle_t nvs_handle;
	uint8_t layer_num = 0;
	esp_err_t error;

	ESP_ERROR_CHECK(nvs_open(LAYER_NAMESPACE, NVS_READWRITE, &nvs_handle));

	error = nvs_get_u8(nvs_handle, LAYER_NUM_KEY, &layer_num);
	switch (error)
	{
		case ESP_ERR_NVS_NOT_FOUND:
			ESP_LOGE(TAG, "Value not set yet. Running routine to write default values");
			nvs_write_default_layers(nvs_handle);
			ESP_ERROR_CHECK(nvs_get_u8(nvs_handle, LAYER_NUM_KEY, &layer_num));
			break;
		case ESP_OK:
			ESP_LOGI(TAG, "Layers in memory: %d", layer_num);
			break;
		default:
			ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(error));
			break;
	}

	nvs_close(nvs_handle);
	return layer_num;
}

/**
 * @brief Read and returns the list of layers
 * 
 * @param list list of the nvs keynames of the layers in order.
 */
void nvs_read_list_layers(layer_list_def * list)
{
	ESP_LOGI(TAG, "Read layer list from memory");
	nvs_handle_t nvs_handle;
	esp_err_t error;

	ESP_ERROR_CHECK(nvs_open(LAYER_NAMESPACE, NVS_READWRITE, &nvs_handle));

	size_t blob_size = sizeof(layer_list_def);
	error = nvs_get_blob(	nvs_handle, 
							LAYER_LIST_KEY, 
							(void *)list, 
							&blob_size
						);
	switch (error)
	{
		case ESP_ERR_NVS_NOT_FOUND:
			ESP_LOGE(TAG, "Value not set yet. Running routine to write default values");
			nvs_write_default_layers(nvs_handle);

			ESP_ERROR_CHECK(nvs_get_blob(	nvs_handle, 
											LAYER_LIST_KEY,
											(void *)list, 
											&blob_size
										));
			break;
		case ESP_OK:
			ESP_LOGI(TAG, "Layer list retrieve correctly");
			break;
		default:
			ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(error));
			break;
	}

	nvs_close(nvs_handle);
}

/**
 * @brief Read all the available layers stored in the memory
 *
 * @param layers_array
 */
void nvs_read_layers(dd_layer *layers_array)
{
	ESP_LOGI(TAG, "READ LAYERS");
	
	nvs_handle_t nvs_layer_handle;
	uint8_t layer_num;
	size_t dd_layer_size = sizeof(dd_layer);

	esp_err_t res;

	layer_list_def layer_list;

	ESP_ERROR_CHECK(nvs_open(LAYER_NAMESPACE, NVS_READWRITE, &nvs_layer_handle));

	res = nvs_get_u8(nvs_layer_handle, LAYER_NUM_KEY, &layer_num);
	switch (res)
	{
		case ESP_ERR_NVS_NOT_FOUND:
			ESP_LOGE(TAG, "Value not set yet. Running routine to write default values");
			nvs_write_default_layers(nvs_layer_handle);
			break;
		case ESP_OK:
			ESP_LOGI(TAG, "Layers in memory: %d", layer_num);
			break;
		default:
			ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(res));
			break;
	}
	
	size_t blob_size = sizeof(layer_list_def);
	ESP_ERROR_CHECK(nvs_get_blob(	nvs_layer_handle, 
									LAYER_LIST_KEY, 
									(void *)&layer_list, 
									&blob_size
								));

	for (int i = 0; i < layer_num; i++)
	{
		
		res = nvs_get_blob(nvs_layer_handle, layer_list[i], (void *)&layers_array[i], &dd_layer_size);
		ESP_LOGI(TAG, "Reading layer with key %s, name: %s, and uuid %s",layer_list[i], layers_array[i].name, layers_array[i].uuid_str);
		if (res != ESP_OK)
		{
			ESP_LOGE(TAG, "Error (%s) reading layer %s!\n", esp_err_to_name(res), layer_list[i]);
		}
	}
	nvs_close(nvs_layer_handle);
}

/**
 * @brief Write default layers
 *
 * @param nvs_handle
 */
void nvs_write_default_layers(nvs_handle_t nvs_handle)
{
	// Set the number of layers (by default LAYERS)
	ESP_ERROR_CHECK(nvs_set_u8(nvs_handle, LAYER_NUM_KEY, LAYERS));
	layer_list_def layer_list = {{0}};
	size_t blob_size = sizeof(dd_layer);

	// Store each of the default layers
	for (int i = 0; i < LAYERS; i++)
	{
		sprintf(layer_list[i], "l_%s", default_layouts[i]->uuid_str);
		ESP_LOGI(TAG,"Storing layer %s with uuid %s in memory key %s", default_layouts[i]->name ,default_layouts[i]->uuid_str,layer_list[i]);

		ESP_ERROR_CHECK(nvs_set_blob(	nvs_handle, 
										layer_list[i], 
										(void *)default_layouts[i], 
										blob_size
									));
		ESP_ERROR_CHECK(nvs_commit(nvs_handle));
	}

	// Store the array list
	blob_size = sizeof(layer_list_def);
	ESP_ERROR_CHECK(nvs_set_blob(	nvs_handle,
									LAYER_LIST_KEY, 
									(void *)(&layer_list),
									blob_size
								));

	// Commit memory to make sure everything is stored.
	ESP_ERROR_CHECK(nvs_commit(nvs_handle));
}

/**
 * @brief Overwrite elements to go to the original declaration.
 *
 * @return esp_err_t
 */
esp_err_t nvs_restore_default_layers(void)
{
	nvs_handle_t nvs_handle;
	esp_err_t error;
	error = nvs_open(LAYER_NAMESPACE, NVS_READWRITE, &nvs_handle);
	//TODO: Before calling nvs_write_default_layers, i should erase everything in the memory
	// to maker sure there is not a leackage.
	if (error == ESP_OK)
	{
		nvs_write_default_layers(nvs_handle);
		nvs_close(nvs_handle);
		nvs_load_layouts();
	}
	else
	{
		ESP_LOGE(TAG, "Error (%s) opening NVS Namespace!: \n", esp_err_to_name(error));
		return error;
	}

	return ESP_OK;
}

/**
 * @brief Write new layer
 *
 * @param layer
 * @param layer_pos layer position in the list
 * @return esp_err_t
 */
esp_err_t nvs_write_layer(dd_layer * layer, uint8_t layer_pos) 
{
	nvs_handle_t nvs_layer_handle;
	layer_list_def layer_list;

	uint8_t layer_num = nvs_read_num_layers();
	nvs_read_list_layers(&layer_list);

	for(int i=0; i<layer_num;i++)
	{
		ESP_LOGE(TAG,"LAYER %i = %s", i, layer_list[i]);
	}

	ESP_ERROR_CHECK(nvs_open(LAYER_NAMESPACE, NVS_READWRITE, &nvs_layer_handle));

	ESP_ERROR_CHECK(nvs_set_blob(	nvs_layer_handle, 
									layer_list[layer_pos], 
									(void *)layer, 
									sizeof(dd_layer)
								));

	ESP_ERROR_CHECK(nvs_commit(nvs_layer_handle));

	nvs_close(nvs_layer_handle);


	nvs_update_layout_position(); 
	nvs_load_layouts();

	return ESP_OK;
}

/**
 * @brief
 *
 * @param layer
 * @return esp_err_t
 */
esp_err_t nvs_create_new_layer(dd_layer layer) //TODO: test
{
	nvs_handle_t nvs_handle;
	uint8_t layer_num;
	layer_list_def current_list;
	dd_layer aux_layer = layer;

	ESP_ERROR_CHECK(nvs_open(	LAYER_NAMESPACE, 
								NVS_READWRITE, 
								&nvs_handle));

	// Read layer list
	size_t layer_list_size = sizeof(layer_list_def);
	ESP_ERROR_CHECK(nvs_get_blob(	nvs_handle, 
									LAYER_LIST_KEY, 
									(void *)&current_list, 
									&layer_list_size
								));

	// Read layer number
	ESP_ERROR_CHECK(nvs_get_u8(	nvs_handle, 
								LAYER_NUM_KEY, 
								&layer_num));
	
	// Check if layer_num + 1 will overflow the layer list
	if (layer_num + 1 > MAX_LAYOUT_NUMBER)
	{
		return ESP_ERR_NO_MEM;
	}
	
	//Add layer id into list
	sprintf(current_list[layer_num], "l_%s", aux_layer.uuid_str);

	// Store new layer with new key
	ESP_ERROR_CHECK(nvs_set_blob(	nvs_handle, 
									current_list[layer_num], 
									(void *)&aux_layer, 
									sizeof(dd_layer)));

	// Increase layer num
	layer_num++;

	// Save number of layers
	ESP_ERROR_CHECK(nvs_set_u8(	nvs_handle, 
								LAYER_NUM_KEY, 
								layer_num));

	// Save layer list
	ESP_ERROR_CHECK(nvs_set_blob(	nvs_handle, 
									LAYER_LIST_KEY, 
									(void *)&current_list, 
									sizeof(layer_list_def)
								));
	
	ESP_ERROR_CHECK(nvs_commit(nvs_handle));
	
	nvs_close(nvs_handle);
	
	nvs_update_layout_position(); 
	nvs_load_layouts();

	return ESP_OK;
}

/**
 * @brief
 *
 * @param delete_layer_num
 * @return esp_err_t
 */
esp_err_t nvs_delete_layer(uint8_t delete_layer_num)
{
	nvs_handle_t nvs_handle;
	uint8_t layer_num;
	layer_list_def current_list;

	ESP_ERROR_CHECK(nvs_open(	LAYER_NAMESPACE, 
								NVS_READWRITE, 
								&nvs_handle));

	// Read layer list
	size_t layer_list_size = sizeof(layer_list_def);
	ESP_ERROR_CHECK(nvs_get_blob(	nvs_handle, 
									LAYER_LIST_KEY, 
									(void *)&current_list, 
									&layer_list_size
								));

	// Read layer number
	ESP_ERROR_CHECK(nvs_get_u8(	nvs_handle, 
								LAYER_NUM_KEY, 
								&layer_num));
	
	// Erase key
	ESP_ERROR_CHECK(nvs_erase_key(	nvs_handle,
									current_list[delete_layer_num]
								));

	// Decrease layer num
	layer_num--;

	// Remove item in list and reorganize it.
	for(int i=delete_layer_num; i<layer_num; i++)
	{
		strcpy(current_list[i],current_list[i+1]);
	}

	strcpy(current_list[layer_num],"");
	

	// Save number of layers
	ESP_ERROR_CHECK(nvs_set_u8(	nvs_handle, 
								LAYER_NUM_KEY, 
								layer_num));

	// Save layer list
	ESP_ERROR_CHECK(nvs_set_blob(	nvs_handle, 
									LAYER_LIST_KEY, 
									(void *)&current_list, 
									sizeof(layer_list_def)
								));
	
	ESP_ERROR_CHECK(nvs_commit(nvs_handle));
	
	nvs_close(nvs_handle);
	
	nvs_load_layouts();

	return ESP_OK;
}

/**
 * @brief Update position of layers, putting the ones that are not enabled at the end.
 * 
 * @return esp_err_t 
 */
esp_err_t nvs_update_layout_position(void) 
{
	layer_list_def layer_list;
	layer_list_def new_layer_list;
	uint8_t aux_counter = 0;
	uint8_t layer_num;
	nvs_handle_t nvs_handle;

	dd_layer * aux_layers;

	
	layer_num = nvs_read_num_layers();
	nvs_read_list_layers(&layer_list);

	aux_layers = pvPortMalloc(layer_num * sizeof(dd_layer));
	nvs_read_layers(aux_layers);
	

	for (int i = 0; i < layer_num; i++)
	{
		if (aux_layers[i].active == true)
		{
			strcpy(new_layer_list[aux_counter], layer_list[i]);
			aux_counter++;
		}
	}
	for (int i = 0; i < (layer_num); i++)
	{
		if (aux_layers[i].active == false)
		{
			strcpy(new_layer_list[aux_counter], layer_list[i]);
			aux_counter++;
		}
	}

	// Finally store the new list
	ESP_ERROR_CHECK(nvs_open(LAYER_NAMESPACE, NVS_READWRITE, &nvs_handle));

	// Store the array list
	size_t blob_size = sizeof(layer_list_def);
	ESP_ERROR_CHECK(nvs_set_blob(	nvs_handle,
									LAYER_LIST_KEY, 
									(void *)(&new_layer_list),
									blob_size
								));

	// Commit memory to make sure everything is stored.
	ESP_ERROR_CHECK(nvs_commit(nvs_handle));

	vPortFree(aux_layers);
	return ESP_OK;
}

/**
 * @brief load the layouts from nvs
 *
 */
void nvs_load_layouts(void)
{

	ESP_LOGI("NVS_TAG", "LOADING LAYOUTS");
	uint8_t layer_num = nvs_read_num_layers();
	
	vPortFree(key_layouts);
	key_layouts = pvPortMalloc(layer_num * sizeof(dd_layer));

	nvs_read_layers(key_layouts);
	

	for (int i = 0; i < layer_num; i++)
	{
		ESP_LOGI("NVS_TAG", "LAYER NAME %s, uuid: %s", key_layouts[i].name,key_layouts[i].uuid_str);
	}
	nvs_check_memory_status();
	ESP_LOGI(NVS_TAG, "Layouts loaded");
}

/*******
 * MACROS
 */

void nvs_macros_state(void)
{
	// Example of nvs_get_used_entry_count() to get amount of all key-value pairs in one namespace:
	nvs_handle_t handle;
	nvs_open(MACROS_NAMESPACE, NVS_READWRITE, &handle);
	size_t used_entries;
	// size_t total_entries_namespace;
	if (nvs_get_used_entry_count(handle, &used_entries) == ESP_OK)
	{
		// the total number of entries occupied by the namespace
		// total_entries_namespace = used_entries + 1;
		used_entries++;
	}
	ESP_LOGI("MACROS", "Total entries in namespace: %zu", used_entries);
	nvs_close(handle);
}

/**
 * @brief load the macros from nvs
 *
 */
void nvs_load_macros(void)
{
	ESP_LOGI("--", "LOADING USER MACROS");
	nvs_handle_t nvs_handle;
	size_t dd_macros_size = sizeof(dd_macros);
	esp_err_t error;
	esp_err_t res;
	uint8_t macro_num = 0;
	char macro_key[10];
	error = nvs_open(MACROS_NAMESPACE, NVS_READWRITE, &nvs_handle);
	if (error == ESP_OK)
	{
		ESP_LOGI("MACROS", "MACROS_NAMESPACE OK");
	}
	else
	{
		ESP_LOGE("MACROS", "Error (%s) opening NVS Namespace!: \n", esp_err_to_name(error));
	}

	error = nvs_get_u8(nvs_handle, MACROS_KEY, &macro_num);

	switch (error)
	{
	case ESP_ERR_NVS_NOT_FOUND:
		ESP_LOGE("--", "Value not set yet. Running routine to write default values");
		nvs_write_default_macros(nvs_handle);
		ESP_ERROR_CHECK(nvs_get_u8(nvs_handle, MACROS_KEY, &macro_num));
		break;
	case ESP_OK:

		ESP_LOGI("--", "%d Macros loaded", macro_num);

		break;
	default:
		ESP_LOGE("--", "Error (%s) opening NVS handle!\n", esp_err_to_name(error));
		break;
	}

	user_macros = pvPortMalloc((macro_num + 1) * sizeof(dd_macros)); //TODO: here is a malloc, but there is never a free.
																	 // if that's ok. I guess so because this function is just called once.

	for (int i = 0; i < macro_num; i++)
	{
		sprintf(macro_key, "macro_%hu", (i + 1));
		res = nvs_get_blob(nvs_handle, macro_key, (void *)&user_macros[i], &dd_macros_size);
		if (res != ESP_OK)
		{
			ESP_LOGE("++", "Error (%s) reading Macro %s!\n", esp_err_to_name(res), macro_key);
		}
	}
	total_macros = macro_num;
	nvs_close(nvs_handle);
	nvs_check_memory_status();
	nvs_macros_state();
}

esp_err_t nvs_create_new_macro(dd_macros macro)
{
	nvs_handle_t nvs_handle;
	esp_err_t error;
	uint8_t macro_num;
	char macro_key[10];
	char temp_key[10];

	if (macro.keycode > MACRO_HOLD_MAX_VAL)
	{
		return ESP_ERR_INVALID_ARG;
	}

	error = nvs_open(MACROS_NAMESPACE, NVS_READWRITE, &nvs_handle);
	if (error == ESP_OK)
	{
		ESP_LOGI("--", "MACROS_NAMESPACE OK");
	}
	else
	{
		ESP_LOGE("MACROS", "Error (%s) opening NVS Namespace!: \n", esp_err_to_name(error));
		nvs_close(nvs_handle);
		return error;
	}

	error = nvs_get_u8(nvs_handle, MACROS_KEY, &macro_num);
	if (error == ESP_OK)
	{
		ESP_LOGI("--", "MACRO KEY FOUND ---OK");
		ESP_LOGI("--", "MACRO QTY %d", macro_num);
	}
	else
	{
		ESP_LOGE("--", "Error (%s) READING KEY!: \n", esp_err_to_name(error));
		nvs_close(nvs_handle);
		return error;
	}
	int i = 0;
	macro_num++;
	if (macro_num > (MACRO_HOLD_MAX_VAL - MACRO_BASE_VAL))
	{
		nvs_close(nvs_handle);
		return ESP_ERR_NVS_NOT_ENOUGH_SPACE;
	}
	dd_macros *temp_macro = pvPortMalloc((macro_num + 1) * sizeof(dd_macros));
	user_macros = realloc(user_macros, (macro_num + 1) * sizeof(dd_macros));

	size_t dd_macros_size = sizeof(dd_macros);

	for (i = 0; i < macro_num - 1; i++)
	{
		sprintf(temp_key, "macro_%d", (i + 1));
		error = nvs_get_blob(nvs_handle, temp_key, (void *)&temp_macro[i], &dd_macros_size);
		if (error != ESP_OK)
		{
			ESP_LOGE("MACROS", "Error (%s) reading Macro %s!\n", esp_err_to_name(error), temp_key);
			vPortFree(temp_macro);
			nvs_close(nvs_handle);
			return error;
		}
	}

	temp_macro[i++] = macro;

	ESP_ERROR_CHECK(nvs_set_u8(nvs_handle, MACROS_KEY, macro_num));
	ESP_ERROR_CHECK(nvs_commit(nvs_handle));

	for (i = 0; i < macro_num; i++)
	{
		sprintf(macro_key, "macro_%hu", (i + 1));
		error = nvs_set_blob(nvs_handle, macro_key, (void *)&temp_macro[i], dd_macros_size);
		if (error != ESP_OK)
		{
			ESP_LOGE("MACROS", "Error (%s) saving Macro %s!\n", esp_err_to_name(error), macro_key);
			vPortFree(temp_macro);
			nvs_close(nvs_handle);
			return error;
		}
		// ESP_LOGI("-", "%s  Writed", macro_key);
		// ESP_ERROR_CHECK(nvs_commit(nvs_handle));
		user_macros[i] = temp_macro[i];
	}
	ESP_ERROR_CHECK(nvs_commit(nvs_handle));
	total_macros = macro_num;
	nvs_close(nvs_handle);
	vPortFree(temp_macro);
	return ESP_OK;
}

esp_err_t nvs_update_macro(dd_macros macro)
{
	ESP_LOGI(NVS_TAG, "UPDATING MACRO");
	nvs_handle_t nvs_handle;
	dd_macros temp;
	esp_err_t error;
	size_t dd_macros_size = sizeof(dd_macros);
	char macro_key[10];
	ESP_ERROR_CHECK(nvs_open(MACROS_NAMESPACE, NVS_READWRITE, &nvs_handle));

	sprintf(macro_key, "macro_%hu", (1 + macro.keycode - MACRO_BASE_VAL));
	ESP_LOGI(NVS_TAG, "Reading  %s", macro_key);
	error = nvs_get_blob(nvs_handle, macro_key, (void *)&temp, &dd_macros_size);
	if (error != ESP_OK)
	{
		ESP_LOGE(NVS_TAG, "Error (%s) reading Macro %s!\n", esp_err_to_name(error), macro_key);
		nvs_close(nvs_handle);
		return error;
	}
	else
	{
		ESP_LOGI(NVS_TAG, "Macro Name  %s, Macro Keycode %d", temp.name, temp.keycode);
	}

	ESP_LOGI(NVS_TAG, "updating  %s", macro_key);
	ESP_ERROR_CHECK(nvs_set_blob(nvs_handle, macro_key, (void *)&macro, sizeof(dd_macros)));
	ESP_ERROR_CHECK(nvs_commit(nvs_handle));
	nvs_close(nvs_handle);
	nvs_load_macros();
	return ESP_OK;
}

esp_err_t nvs_delete_macro(dd_macros macro)
{
	nvs_handle_t nvs_handle;
	char macro_key[10];
	ESP_ERROR_CHECK(nvs_open(LAYER_NAMESPACE, NVS_READWRITE, &nvs_handle));
	sprintf(macro_key, "macro_%hu", (macro.keycode - MACRO_BASE_VAL));
	ESP_ERROR_CHECK(nvs_set_blob(nvs_handle, macro_key, (void *)&macro, sizeof(dd_macros)));
	ESP_ERROR_CHECK(nvs_commit(nvs_handle));
	nvs_close(nvs_handle);
	nvs_load_macros();

	return ESP_OK;
}

esp_err_t nvs_restore_default_macros(void)
{
	nvs_handle_t nvs_handle;
	nvs_handle_t new_nvs_handle;
	esp_err_t error;
	error = nvs_open(MACROS_NAMESPACE, NVS_READWRITE, &nvs_handle);
	if (error == ESP_OK)
	{
		ESP_LOGI("--", "MACROS_NAMESPACE OK");
		ESP_ERROR_CHECK(nvs_erase_key(nvs_handle, MACROS_KEY));
		ESP_ERROR_CHECK(nvs_erase_all(nvs_handle));
		ESP_ERROR_CHECK(nvs_commit(nvs_handle));
		nvs_close(nvs_handle);

		ESP_ERROR_CHECK(nvs_open(MACROS_NAMESPACE, NVS_READWRITE, &new_nvs_handle));
		nvs_write_default_macros(new_nvs_handle);
		nvs_close(new_nvs_handle);
		nvs_load_macros();
	}
	else
	{
		ESP_LOGE("--", "Error (%s) opening NVS Namespace!: \n", esp_err_to_name(error));
		nvs_close(nvs_handle);
		return error;
	}

	return ESP_OK;
}

esp_err_t nvs_write_default_macros(nvs_handle_t nvs_handle)
{
	ESP_LOGI(TAG, "WRITING DEFAULT MACROS");
	char macro_key[10];
	ESP_ERROR_CHECK(nvs_set_u8(nvs_handle, MACROS_KEY, MACROS_NUM));
	for (int i = 0; i < MACROS_NUM; i++)
	{
		sprintf(macro_key, "macro_%hu", (i + 1));
		ESP_ERROR_CHECK(nvs_set_blob(nvs_handle, macro_key, (void *)ptr_default_macros[i], sizeof(dd_macros)));
		ESP_ERROR_CHECK(nvs_commit(nvs_handle));
	}

	return ESP_OK;
}


/**********
 * TAPDANCE
*/

void nvs_load_tapdance(void)
{
	ESP_LOGI(TAG, "LOADING USER TAPDANCEs");

	nvs_handle_t nvs_handle;
	size_t dd_tapdance_size = sizeof(dd_tapdance);
	esp_err_t error;
	esp_err_t res;
	uint8_t tapdance_num = 0;
	
	char tapdance_key[10];

	error = nvs_open(TAPDANCE_NAMESPACE, NVS_READWRITE, &nvs_handle);
	if (error == ESP_OK) //TODO: assert instead of print this errors.
	{
		ESP_LOGI(TAG, "TAPDANCE_NAMESPACE OK");
	}
	else
	{
		ESP_LOGE(TAG, "Error (%s) opening NVS Namespace!: \n", esp_err_to_name(error));
	}

	error = nvs_get_u8(nvs_handle, TAPDANCE_NUM_KEY, &tapdance_num);

	switch (error)
	{
	case ESP_ERR_NVS_NOT_FOUND:
		ESP_LOGE("--", "Value not set yet. Running routine to write default values");
		nvs_write_default_tapdance(nvs_handle);
		ESP_ERROR_CHECK(nvs_get_u8(nvs_handle, TAPDANCE_NUM_KEY, &tapdance_num));
		break;
	case ESP_OK:

		ESP_LOGI("--", "%d TapDances loaded", tapdance_num);

		break;
	default:
		ESP_LOGE("--", "Error (%s) opening NVS handle!\n", esp_err_to_name(error));
		break;
	}

	user_tapdance = pvPortMalloc((tapdance_num + 1) * sizeof(dd_macros));
	for (int i = 0; i < tapdance_num; i++)
	{
		sprintf(tapdance_key, "tapdance_%hu", (i + 1));
		res = nvs_get_blob(nvs_handle, tapdance_key, (void *)&user_tapdance[i], &dd_tapdance_size);
		if (res != ESP_OK)
		{
			ESP_LOGE("++", "Error (%s) reading Tapdance %s!\n", esp_err_to_name(res), tapdance_key);
		}
	}
	total_tapdance = tapdance_num;
	nvs_close(nvs_handle);
	nvs_check_memory_status();
	//nvs_macros_state(); TODO: see if its necessary to have one of this for tapdance
}


esp_err_t nvs_write_default_tapdance(nvs_handle_t nvs_handle)
{
	ESP_LOGI(TAG, "WRITING DEFAULT TAPDANCE");
	char tapdance_key[10]; //TODO: Reference a default, not a magic number.
	ESP_ERROR_CHECK(nvs_set_u8(nvs_handle, TAPDANCE_NUM_KEY, TAPDANCE_NUM)); //TODO: Put only max tapdance defined, not total ones.
	for (int i = 0; i < TAPDANCE_NUM; i++)
	{
		sprintf(tapdance_key, "td_%hu", (i + 1));
		ESP_ERROR_CHECK(nvs_set_blob(nvs_handle, tapdance_key, (void *)(&default_tapdance[i]), sizeof(dd_tapdance)));
		ESP_ERROR_CHECK(nvs_commit(nvs_handle));
	}

	return ESP_OK;
}

esp_err_t nvs_update_tapdance(dd_tapdance tapdance)
{
	ESP_LOGI(NVS_TAG, "UPDATING TAPDANCE");
	nvs_handle_t nvs_handle;
	dd_tapdance temp;
	esp_err_t error;
	size_t dd_tapdance_size = sizeof(dd_tapdance);
	char tapdance_key[10]; // TODO: use define instead of magic number.
	ESP_ERROR_CHECK(nvs_open(TAPDANCE_NAMESPACE, NVS_READWRITE, &nvs_handle));

	sprintf(tapdance_key, "macro_%hu", (1 + tapdance.keycode - TAPDANCE_BASE_VAL));
	ESP_LOGI(NVS_TAG, "Reading  %s", tapdance_key);
	error = nvs_get_blob(nvs_handle, tapdance_key, (void *)&temp, &dd_tapdance_size);
	if (error != ESP_OK) // TODO: Assert instead of nice handle.
	{ 
		ESP_LOGE(NVS_TAG, "Error (%s) reading Macro %s!\n", esp_err_to_name(error), tapdance_key);
		nvs_close(nvs_handle);
		return error;
	}
	else
	{
		ESP_LOGI(NVS_TAG, "TapDance Name  %s, TapDance Keycode %d", temp.name, temp.keycode);
	}

	ESP_LOGI(NVS_TAG, "updating  %s", tapdance_key);
	ESP_ERROR_CHECK(nvs_set_blob(nvs_handle, tapdance_key, (void *)&tapdance, sizeof(dd_tapdance)));
	ESP_ERROR_CHECK(nvs_commit(nvs_handle));
	nvs_close(nvs_handle);
	nvs_load_tapdance();
	return ESP_OK;
}


// TODO: Finish TAPDANCE functions   Z             
esp_err_t nvs_create_tapdance(dd_tapdance tapdance);
esp_err_t nvs_delete_tapdance(dd_tapdance tapdance);
esp_err_t nvs_restore_default_tapdance(dd_tapdance tapdance);





esp_err_t nvs_save_led_mode(rgb_mode_t led_mode)
{
	nvs_handle_t nvs_handle;
	esp_err_t error;
	error = nvs_open(LEDMODE_NAMESPACE, NVS_READWRITE, &nvs_handle);
	if (error == ESP_OK)
	{
		// ESP_LOGI("-", "LEDMODE_NAMESPACE OK");
		nvs_set_u8(nvs_handle, "mode", led_mode.mode);
		nvs_set_u8(nvs_handle, "hue", led_mode.H);
		nvs_set_u8(nvs_handle, "saturation", led_mode.S);
		nvs_set_u8(nvs_handle, "value", led_mode.V);
		nvs_set_u8(nvs_handle, "speed", led_mode.speed);
		nvs_set_u8(nvs_handle, "red", led_mode.rgb[0]);
		nvs_set_u8(nvs_handle, "green", led_mode.rgb[1]);
		nvs_set_u8(nvs_handle, "blue", led_mode.rgb[2]);
		nvs_close(nvs_handle);
	}
	else
	{
		ESP_LOGE("-", "Error (%s) opening NVS Namespace!: \n", esp_err_to_name(error));
		nvs_close(nvs_handle);
		return error;
	}

	return ESP_OK;
}

esp_err_t nvs_load_led_mode(rgb_mode_t *led_mode)
{
	nvs_handle_t nvs_handle;
	esp_err_t error;
	error = nvs_open(LEDMODE_NAMESPACE, NVS_READWRITE, &nvs_handle);
	if (error == ESP_OK)
	{
		// ESP_LOGI("-", "LEDMODE_NAMESPACE OK");
		nvs_get_u8(nvs_handle, "mode", &led_mode->mode);
		nvs_get_u8(nvs_handle, "hue", &led_mode->H);
		nvs_get_u8(nvs_handle, "saturation", &led_mode->S);
		nvs_get_u8(nvs_handle, "value", &led_mode->V);
		nvs_get_u8(nvs_handle, "speed", &led_mode->speed);
		nvs_get_u8(nvs_handle, "red", &led_mode->rgb[0]);
		nvs_get_u8(nvs_handle, "green", &led_mode->rgb[1]);
		nvs_get_u8(nvs_handle, "blue", &led_mode->rgb[2]);
		nvs_close(nvs_handle);
	}
	else
	{
		ESP_LOGE("-", "Error (%s) opening NVS Namespace!: \n", esp_err_to_name(error));
		nvs_close(nvs_handle);
		return error;
	}

	return ESP_OK;
}

esp_err_t nvs_load_rgb_color(rgb_mode_t *led_mode)
{
	nvs_handle_t nvs_handle;
	esp_err_t error;
	error = nvs_open(LEDMODE_NAMESPACE, NVS_READWRITE, &nvs_handle);
	if (error == ESP_OK)
	{
		// ESP_LOGI("-", "LEDMODE_NAMESPACE OK");
		nvs_get_u8(nvs_handle, "red", &led_mode->rgb[0]);
		nvs_get_u8(nvs_handle, "green", &led_mode->rgb[1]);
		nvs_get_u8(nvs_handle, "blue", &led_mode->rgb[2]);
		nvs_close(nvs_handle);
	}
	else
	{
		ESP_LOGE("-", "Error (%s) opening NVS Namespace!: \n", esp_err_to_name(error));
		nvs_close(nvs_handle);
		return error;
	}

	return ESP_OK;
}
