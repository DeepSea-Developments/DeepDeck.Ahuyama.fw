/**
 * @file deepdeck_tasks.h
 * @author ElectroNick (nick@dsd.dev)
 * @brief header file for the main FreeRtos tasks on DeepDeck
 * @version 0.1
 * @date 2022-12-08
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef INC_SERVER_NVS_H
#define INC_SERVER_NVS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <keyboard_config.h>
#include "nvs_keymaps.h"
#include "nvs.h"
#include "rgb_led.h"

typedef struct
{
    dd_modtap *item;
    size_t size;
} dd_modtap_lst_t;

typedef struct
{
    dd_tapdance *item;
    size_t size;
} dd_tapdance_lst_t;

typedef struct
{
    dd_layer *item;
    size_t size;
} dd_layer_lst_t;

typedef struct
{
    dd_macros *item;
    size_t size;
} dd_macros_lst_t;

typedef struct
{
    dd_leaderkey *item;
    size_t size;
} dd_leaderkey_lst_t;

// TODO: Document macro NVS functions
void nvs_load_macros(void);
dd_macros_lst_t nvs_get_macros_lst(void);
esp_err_t nvs_restore_default_macros(void);
esp_err_t nvs_create_macros(dd_macros macros);
esp_err_t nvs_update_macros(dd_macros macros);
esp_err_t nvs_delete_macros(uint16_t keycode);

void nvs_load_layer(void);
esp_err_t nvs_restore_default_layer();
dd_layer_lst_t nvs_get_layer_lst(void);
esp_err_t nvs_create_layer(dd_layer layer);
esp_err_t nvs_update_layer(dd_layer layer);
esp_err_t nvs_delete_layer(const char *uuid_str);

void nvs_load_modtap(void);
dd_modtap_lst_t nvs_get_modtap_lst(void);
esp_err_t nvs_restore_default_modtap(void);
esp_err_t nvs_create_modtap(dd_modtap modtap);
esp_err_t nvs_update_modtap(dd_modtap modtap);
esp_err_t nvs_delete_modtap(uint16_t keycode);

void nvs_load_tapdance(void);
dd_tapdance_lst_t nvs_get_tapdance_lst(void);
esp_err_t nvs_restore_default_tapdance(void);
esp_err_t nvs_create_tapdance(dd_tapdance tapdance);
esp_err_t nvs_update_tapdance(dd_tapdance tapdance);
esp_err_t nvs_delete_tapdance(uint16_t keycode);

void nvs_load_leaderkey(void);
dd_leaderkey_lst_t nvs_get_leaderkey_lst(void);
esp_err_t nvs_restore_default_leaderkey(void);
esp_err_t nvs_create_leaderkey(dd_leaderkey leaderkey);
esp_err_t nvs_update_leaderkey(dd_leaderkey leaderkey);
esp_err_t nvs_delete_leaderkey(uint16_t keycode);

// TODO: Document led functions
esp_err_t nvs_save_led_mode(rgb_mode_t led_mode);
esp_err_t nvs_load_led_mode(rgb_mode_t *led_mode);
esp_err_t nvs_load_rgb_color(rgb_mode_t *led_mode);

void nvs_server_auto_erase_nvs(void);
void nvs_server_init(void);

bool macro_keycode_exist(uint16_t keycode);

#endif
