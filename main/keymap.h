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

// A bit different from QMK, default returns you to the first layer, LOWER and raise increase/lower layer by order.
#define DEFAULT 400 // 0x100 ->
#define LOWER 401	// 0x101
#define RAISE 402	// 0x102

extern const char encoder_items_names[ENCODER_SIZE][15];
extern const char gesture_items_names[GESTURE_SIZE][8];

extern uint8_t current_layout;

#endif /* MAIN_KEYMAP_H_ */
