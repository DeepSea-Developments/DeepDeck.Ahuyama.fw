#ifndef KEYMAP_C
#define KEYMAP_C

#include "key_definitions.h"
#include "keyboard_config.h"
#include "keymap.h"
#include "plugins.h"
#include "nvs_keymaps.h"
#include "stdlib.h"



const char encoder_items_names[ENCODER_SIZE][15] = {"cw", "ccw", "single_press", "long_press", "double_press"};
const char gesture_items_names[GESTURE_SIZE][8] = {"up", "down", "left", "right", "near", "far"};

uint8_t current_layout = 0;

#endif
