/**
 * @file nvs_funcs.c
 * @author ElectroNick (nick@dsd.dev)
 * @brief 
 * @version 0.1
 * @date 2022-12-10
 * 
 * @copyright Copyright (c) 2022
 * Based o the code f Gal Zaidenstein.
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
#include <arr_conv.h>

#define NVS_TAG "NVS Storage"

#define KEYMAP_NAMESPACE "keymap_config"
#define ENCODER_NAMESPACE "encoder_config"
#define SLAVE_ENCODER_NAMESPACE "slave_encoder_config"


#include "key_definitions.h"
// #include "keyboard_config.h"
#include "keymap.h"
// #include "plugins.h"

//NameSpaces
#define LAYER_NAMESPACE "layers"

//Keys
#define LAYER_NUM_KEY "layer_num"

dd_layer *key_layouts;
uint8_t layers_num=0;

// Check the number of available entries of the NVS
void nvs_check_memory_status(void)
{
	nvs_stats_t nvs_stats;
	nvs_get_stats(NULL, &nvs_stats);
	ESP_LOGI("NVS Status", "Count: UsedEntries = (%d), FreeEntries = (%d), AllEntries = (%d)",
       nvs_stats.used_entries, nvs_stats.free_entries, nvs_stats.total_entries); 
}

// Read and returns the number of available layers stored
uint8_t nvs_read_num_layers(void)
{
	nvs_handle_t nvs_handle;
	uint8_t layer_num = 0;
	const static char * TAG = "NVS Read#layers";
	
	ESP_ERROR_CHECK(nvs_open(LAYER_NAMESPACE,NVS_READWRITE,&nvs_handle));
	
	esp_err_t res = nvs_get_u8(nvs_handle,LAYER_NUM_KEY,&layer_num);
	switch (res)
    {
    case ESP_ERR_NVS_NOT_FOUND:
      ESP_LOGE(TAG, "Value not set yet. Running routine to write default values");
	  nvs_write_default_layers(nvs_handle);
	  ESP_ERROR_CHECK(nvs_get_u8(nvs_handle,LAYER_NUM_KEY,&layer_num));
    break;
    case ESP_OK:
      ESP_LOGI(TAG, "Layers in memory: %d", layer_num);
    break;
    default:
      ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(res));
    break;
    }

	nvs_close(nvs_handle);
	return layer_num;
}

// Read all the available layers stored in the memory
void nvs_read_layers(dd_layer * layers_array)
{
	nvs_handle_t nvs_layer_handle;
	uint8_t layer_num;
	char layer_num_key[10] = "layer_num";
	const static char * TAG = "Read Layers";
	char layer_key[10];
	size_t dd_layer_size;

	esp_err_t res;

	ESP_ERROR_CHECK(nvs_open(LAYER_NAMESPACE,NVS_READWRITE, &nvs_layer_handle));
	
	res = nvs_get_u8(nvs_layer_handle,layer_num_key,&layer_num);
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
	
	for(int i = 0; i < layer_num; i++)
	{
		sprintf(layer_key, "layer_%d", i);

		res = nvs_get_blob(nvs_layer_handle, layer_key, (void *)&layers_array[i], &dd_layer_size);
		if(res != ESP_OK)
		{
			ESP_LOGE(TAG, "Error (%s) reading layer %s!\n", esp_err_to_name(res), layer_key);
		}
	}
	nvs_close(nvs_layer_handle);
}

// Write default layers
void nvs_write_default_layers(nvs_handle_t nvs_handle)
{
	char layer_num_key[10] = "layer_num";
	char layer_key[10];
	const static char * TAG = "Read Layers";
	uint8_t default_layer_num = LAYERS;
	
	ESP_ERROR_CHECK(nvs_set_u8(nvs_handle,layer_num_key,default_layer_num));

	for(int i = 0; i < default_layer_num; i++)
	{
		sprintf(layer_key, "layer_%d", i);
		
		ESP_ERROR_CHECK(nvs_set_blob(nvs_handle, layer_key, (void *) default_layouts[i], sizeof(dd_layer) ));
		ESP_ERROR_CHECK(nvs_commit(nvs_handle));
	}	
}

esp_err_t nvs_write_layer(dd_layer layer, uint8_t layer_num)
{
	nvs_handle_t nvs_layer_handle;
	char layer_key[10];


	ESP_ERROR_CHECK(nvs_open(LAYER_NAMESPACE,NVS_READWRITE, &nvs_layer_handle));

	sprintf(layer_key, "layer_%d", layer_num);
	ESP_ERROR_CHECK(nvs_set_blob(nvs_layer_handle, layer_key, (void *) &layer, sizeof(dd_layer) ));
	ESP_ERROR_CHECK(nvs_commit(nvs_layer_handle));

	nvs_close(nvs_layer_handle);

	return ESP_OK;
}


//load the layouts from nvs
void nvs_load_layouts(void)
{
	layers_num = nvs_read_num_layers();
	key_layouts = malloc(layers_num*sizeof(dd_layer));
	
	nvs_read_layers(key_layouts);
	nvs_check_memory_status();

	ESP_LOGI("NVS_TAG","Layer names %s, %s, %s",key_layouts[0].name,key_layouts[1].name,key_layouts[2].name);
	ESP_LOGI(NVS_TAG,"Layouts loaded");
}

