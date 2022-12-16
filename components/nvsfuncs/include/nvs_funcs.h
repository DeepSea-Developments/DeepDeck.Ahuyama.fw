/**
 * @file nvs_funcs.h
 * @author ElectroNBick (nick@dsd.dev)
 * @brief functions to load layers from NVS
 * @version 0.1
 * @date 2022-12-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef NVS_FUNCS_H_
#define NVS_FUNCS_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <keyboard_config.h>
#include "nvs_keymaps.h"

#define NVS_CONFIG_OK 1
#define NVS_CONFIG_ERR 0

/**
 * @brief Check the number of entries used and available of the nvs
 * 
 */
void nvs_check_memory_status(void);

/**
 * @brief Read the number of layers in memory
 * 
 * @return uint8_t number of layers in memory
 */
uint8_t nvs_read_num_layers(void);

/**
 * @brief Read layers in memory
 * 
 * @param layers_array array of dd_layer elements (layers in memory)
 */
void nvs_read_layers(dd_layer * layers_array);

/**
 * @brief Write default layers in memory
 * 
 * @param nvs_handle 
 */
void nvs_write_default_layers(nvs_handle_t nvs_handle);


esp_err_t nvs_write_layer(dd_layer layer, uint8_t layer_num);

/**
 * @brief get the layers from memory and load them to for usage
 * 
 */
void nvs_load_layouts(void);

#endif /* NVS_FUNCS_H_ */
