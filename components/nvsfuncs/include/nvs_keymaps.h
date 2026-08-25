/*
 * nvs_keymaps.h
 *
 *  Created on: 19 Sep 2018
 *      Author: gal
 */

#ifndef NVS_KEYMAPS_H_
#define NVS_KEYMAPS_H_
#include <keyboard_config.h>
#include "uuid.h"


//*************new*********

/**
 * @brief A 24 bit colour, for one key or for a whole layer
 */
typedef struct dd_key_color_str {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} dd_key_color_t;

/* Stamped into every dd_layer that carries colour information. A layer blob
 * written before the colour fields existed is shorter than the current struct,
 * so color_ver reads back as 0 and nvs_read_layers() knows to fill in the
 * defaults instead of trusting whatever is in those bytes. Bump this whenever
 * the meaning of the colour fields changes. */
#define DD_LAYER_COLOR_VER 1

typedef struct dd_layer_str {
	char name[MAX_LAYOUT_NAME_LENGTH];                     // Name up to 14 characters
	uint16_t key_map[MATRIX_ROWS][MATRIX_COLS];            // Key maps
	char key_map_names[MATRIX_ROWS][MATRIX_COLS][7];       // Name of each key up to 6 characters
	uint16_t left_encoder_map[ENCODER_SIZE];               // Map ofr left encoder
	uint16_t right_encoder_map[ENCODER_SIZE];              // Map for rigth encoder
	uint16_t gesture_map[GESTURE_SIZE];                    // Map for gesture sensor
	bool active;
	char uuid_str[SHORT_UUID_STR_LEN];
	uint8_t color_ver;                                     // DD_LAYER_COLOR_VER, or 0 from an older firmware
	dd_key_color_t layer_color;                            // Whole pad colour, used by RGB_MODE_LAYER_COLOR
	dd_key_color_t key_map_colors[MATRIX_ROWS][MATRIX_COLS]; // Per key colour. All zero means "use layer_color"
} dd_layer;


// typedef struct dd_macros_str {
// 	uint16_t key[USER_MACROS_NUM][MACRO_LEN];  				//200 keys ---> MACRO_LEN = 5       
// 	char name[USER_MACROS_NUM][USER_MACRO_NAME_LEN];       //  200 macros ---> Name of each macro up to 32 characters
// 	int keycode[USER_MACROS_NUM];
// } dd_macros;

typedef struct dd_macros_str {
	uint16_t key[MACRO_LEN];  				//200 keys ---> MACRO_LEN = 5       
	char name[USER_MACRO_NAME_LEN];       //  200 macros ---> Name of each macro up to 32 characters
	int keycode;
} dd_macros;


extern dd_macros *user_macros;
extern dd_layer *key_layouts;

//amount of arrays
extern uint8_t layers_num;
extern int total_macros;

#endif /* NVS_KEYMAPS_H_ */



