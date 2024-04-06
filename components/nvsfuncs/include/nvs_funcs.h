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
 * @brief Check the number of entries used and available of the nvs
 *
 */
void nvs_check_memory_status(void);

typedef bool (*selector_t)(const void *item);

esp_err_t nvs_list_init(const char *list_name);
esp_err_t nvs_list_initialized(const char *list_name);
esp_err_t nvs_list_default(const char *list_name, void *list, size_t list_len, size_t item_size);
esp_err_t nvs_list_auto_default(const char *list_name, void *list, size_t list_len, size_t item_size);
esp_err_t nvs_list_load(const char *list_name, void **list, size_t *list_len, size_t item_size);
esp_err_t nvs_list_add_item(const char *list_name, void *item, size_t item_size);
esp_err_t nvs_list_update(const char *list_name, size_t index, void *item, size_t item_size);
esp_err_t nvs_list_delete_item(const char *list_name, size_t index);
esp_err_t nvs_list_organize(const char *list_name, size_t item_size, selector_t selector_cb);
esp_err_t nvs_list_get_len(const char *list_name, size_t *lst_len);
esp_err_t nvs_list_load_item(const char *list_name, size_t index, void **item, size_t item_size);

esp_err_t nvs_delete_item(const char *namespace, const char *item_name);
esp_err_t nvs_item_store(const char *namespace, const char *item_name, void *item, size_t item_size);
esp_err_t nvs_item_load(const char *namespace, const char *item_name, void **item, size_t *item_size);

#endif /* NVS_FUNCS_H_ */
