/**
 * @file nvs_funcs.h
 * @author ElectroNick (nick@dsd.dev)
 * @brief
 * @version 0.1
 * @date 2023-11-06
 *
 * @copyright Copyright (c) 2023
 * Based on the code of Gal Zaidenstein.
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

#ifndef NVS_FUNCS_H_
#define NVS_FUNCS_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <keyboard_config.h>
#include "nvs_keymaps.h"
#include "nvs.h"
#include "rgb_led.h"

#define NVS_CONFIG_OK 1
#define NVS_CONFIG_ERR 0

/**
 * @brief List of available layers in order
 * 
 */
typedef char layer_list_def[MAX_LAYER][NVS_NS_NAME_MAX_SIZE]; 


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
void nvs_read_layers(dd_layer *layers_array);

/**
 * @brief Read and returns the list of layers
 * 
 * @param list list of the nvs keynames of the layers in order.
 */
void nvs_read_list_layers(layer_list_def * list);

/**
 * @brief Write default layers in memory
 *
 * @param nvs_handle
 */
void nvs_write_default_layers(nvs_handle_t nvs_handle);

/**
 * @brief
 *
 * @param layer
 * @param layer_num
 * @return esp_err_t
 */
esp_err_t nvs_write_layer(dd_layer * layer, uint8_t layer_pos);

/**
 * @brief
 *
 * @param layer
 * @return esp_err_t
 */
esp_err_t nvs_create_new_layer(dd_layer layer);

/**
 * @brief
 *
 * @param delete_layer_num
 * @return esp_err_t
 */
esp_err_t nvs_delete_layer(uint8_t delete_layer_num);

/**
 * @brief
 *
 * @return esp_rr_t
 */
esp_err_t nvs_restore_default_layers();

/**
 * @brief get the layers from memory and load them to for usage
 *
 */
void nvs_load_layouts(void);

/**
 * @brief
 *
 */
esp_err_t nvs_update_layout_position(void);

// TODO: Document macro NVS functions
void nvs_load_macros(void);
esp_err_t nvs_write_default_macros(nvs_handle_t nvs_handle);
esp_err_t nvs_create_new_macro(dd_macros macros);
esp_err_t nvs_update_macro(dd_macros macro);
esp_err_t nvs_delete_macro(dd_macros macro);
esp_err_t nvs_restore_default_macros(void);

void nvs_load_tapdance(void);
esp_err_t nvs_write_default_tapdance(nvs_handle_t nvs_handle);
esp_err_t nvs_create_tapdance(dd_tapdance tapdance);
esp_err_t nvs_update_tapdance(dd_tapdance tapdance);
esp_err_t nvs_delete_tapdance(dd_tapdance tapdance);
esp_err_t nvs_restore_default_tapdance(dd_tapdance tapdance);

// TODO: Document led functions
esp_err_t nvs_save_led_mode(rgb_mode_t led_mode);
esp_err_t nvs_load_led_mode(rgb_mode_t *led_mode);
esp_err_t nvs_load_rgb_color(rgb_mode_t *led_mode);

#endif /* NVS_FUNCS_H_ */
