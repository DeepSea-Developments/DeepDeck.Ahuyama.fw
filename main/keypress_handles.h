
#ifndef INC_KEYPRESS_HANDLES_H
#define INC_KEYPRESS_HANDLES_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "keyboard_config.h"



void layer_adjust(uint16_t keycode);
void media_control_release(uint16_t keycode);
void media_control_send(uint16_t keycode);
uint16_t check_modifier(uint16_t key);


#endif // INC_KEYPRESS_HANDLES_H