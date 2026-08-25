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
#include "nvs.h"
#include "rgb_led.h"

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
void nvs_read_layers(dd_layer *layers_array);

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
esp_err_t nvs_write_layer(dd_layer layer, uint8_t layer_num);

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

void nvs_load_macros(void);
esp_err_t nvs_write_default_macros(nvs_handle_t nvs_handle);
esp_err_t nvs_create_new_macro(dd_macros macros);
esp_err_t nvs_update_macro(dd_macros macro);
esp_err_t nvs_delete_macro(dd_macros macro);
esp_err_t nvs_restore_default_macros(void);


/**
 * @brief Give a layer the default colours for its position
 *
 * Used for layers that carry no colour information of their own: read back from
 * a firmware older than DD_LAYER_COLOR_VER, or created by a client that does
 * not send colours. Sets layer_color from a small palette indexed by position,
 * clears the per key colours so they inherit it, and stamps color_ver.
 *
 * @param layer
 * @param index layer position, used to pick a colour from the palette
 */
void dd_layer_set_default_colors(dd_layer *layer, uint8_t index);

esp_err_t nvs_save_led_mode(rgb_mode_t led_mode);
esp_err_t nvs_load_led_mode(rgb_mode_t *led_mode);
esp_err_t nvs_load_rgb_color(rgb_mode_t *led_mode);

/**
 * @brief Store the screensaver timeout, in seconds (0 = screensaver off)
 *
 * @param seconds idle time before the OLED is blanked
 * @return esp_err_t
 */
esp_err_t nvs_save_screensaver_secs(uint16_t seconds);

/**
 * @brief Read the screensaver timeout, in seconds
 *
 * Falls back to the older whole-minutes key if that is all that is stored.
 *
 * @param seconds left untouched if nothing has been stored yet, so the caller
 *                keeps its compiled-in default
 * @return esp_err_t ESP_ERR_NVS_NOT_FOUND when never saved
 */
esp_err_t nvs_load_screensaver_secs(uint16_t *seconds);

#endif /* NVS_FUNCS_H_ */
