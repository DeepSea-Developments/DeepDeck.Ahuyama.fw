
#ifndef INC_KEYPRESS_HANDLES_H
#define INC_KEYPRESS_HANDLES_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "keyboard_config.h"


#include "nvs_keymaps.h"
#include "keys.h"

void layer_adjust(uint16_t keycode);
void media_control_release(uint16_t keycode);
void media_control_send(uint16_t keycode);
uint16_t check_modifier(uint16_t key);

void keys_get_report_from_event(dd_layer *keymap, keys_event_struct_t key_event,uint8_t * report_state);	


#endif // INC_KEYPRESS_HANDLES_H