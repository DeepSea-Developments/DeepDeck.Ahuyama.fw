/*
 * nvs_keymaps.h
 *
 *  Created on: 19 Sep 2018
 *      Author: gal
 */

#ifndef NVS_KEYMAPS_H_
#define NVS_KEYMAPS_H_
#include <keyboard_config.h>


//*************new*********

typedef struct dd_layer_str {
	char name[MAX_LAYOUT_NAME_LENGTH];                     // Name up to 14 characters
	uint16_t key_map[MATRIX_ROWS][MATRIX_COLS];            // Key maps
	char key_map_names[MATRIX_ROWS][MATRIX_COLS][7];       // Name of each key up to 6 characters
	uint16_t left_encoder_map[ENCODER_SIZE];               // Map ofr left encoder
	uint16_t right_encoder_map[ENCODER_SIZE];              // Map for rigth encoder
	uint16_t gesture_map[GESTURE_SIZE];                    // Map for gesture sensor
} dd_layer;

extern dd_layer *key_layouts;

//amount of arrays
extern uint8_t layers_num;


#endif /* NVS_KEYMAPS_H_ */
