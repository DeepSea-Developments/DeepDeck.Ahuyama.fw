/**
 * @file keymap.h
 * @author ElectroNick (nick@dsd.dev)
 * @brief 
 * @version 0.1
 * @date 2022-12-11
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef MAIN_KEYMAP_H_
#define MAIN_KEYMAP_H_

#include "keyboard_config.h"
#include "nvs_keymaps.h"






extern dd_layer *default_layouts[LAYERS];
extern dd_macros default_macros[MACROS_NUM];
extern dd_macros *ptr_default_macros[MACROS_NUM];

extern dd_tapdance default_tapdance[TAPDANCE_NUM];


extern char encoder_items_names[ENCODER_SIZE][15];
extern char gesture_items_names[GESTURE_SIZE][8] ;

extern uint8_t current_layout;
// void init_keymap();
void generate_uuid();
void init_default_macros();

#endif /* MAIN_KEYMAP_H_ */
