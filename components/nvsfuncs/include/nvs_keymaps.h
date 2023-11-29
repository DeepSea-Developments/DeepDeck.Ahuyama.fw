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

#ifndef NVS_KEYMAPS_H_
#define NVS_KEYMAPS_H_
#include <keyboard_config.h>
#include "uuid.h"
#include "nvs.h"


#define MG_NAME_LENGTH 13

#define LK_NAME_LENGTH 13
#define LK_MAX_SEQUENCE 6

#define MT_NAME_LENGTH 13

#define TD_NAME_LENGTH 13
#define TD_MAX_TAP_SEQUENCES 6

#define MACRO_SHORT_NAME_LENGTH 7
#define MACRO_LONG_NAME_LENGTH 13


/**
 * @brief Structure to hold DeepDeck layers
 * 
 */
typedef struct dd_layer_str {
	char name[MAX_LAYOUT_NAME_LENGTH];                     // Name up to 14 characters
	uint16_t key_map[MATRIX_ROWS][MATRIX_COLS];            // Key maps
	char key_map_names[MATRIX_ROWS][MATRIX_COLS][7];       // Name of each key up to 6 characters
	uint16_t left_encoder_map[ENCODER_SIZE];               // Map ofr left encoder
	uint16_t right_encoder_map[ENCODER_SIZE];              // Map for right encoder
	uint16_t gesture_map[GESTURE_SIZE];                    // Map for gesture sensor
	bool active;
	char uuid_str[SHORT_UUID_STR_LEN];
} dd_layer;

/**
 * @brief List of available layers in order
 * 
 */
typedef char layer_list_def[MAX_LAYER][NVS_NS_NAME_MAX_SIZE]; 

/**
 * @brief structure to hold tapdance actions. 
 * 
 */
typedef struct dd_tapdance_str {
	char name[TD_NAME_LENGTH];       
	uint8_t tap_list[TAPDANCE_LEN];
	uint16_t keycode_list[TAPDANCE_LEN];
	uint16_t keycode;
} dd_tapdance;

/**
 * @brief List of available tapdance in order
 * 
 */
typedef char tapdance_list_def[MAX_TAPDANCE][NVS_NS_NAME_MAX_SIZE]; 


typedef struct dd_modtap_str {
	char name[MT_NAME_LENGTH];
	uint16_t keycode_short;			// Keycode when short pressed
	uint16_t keycode_long;			// Keycode when long pressed
	uint16_t keycode;				// Keycode that triggers this modtap
}dd_modtap;

/**
 * @brief List of available tapdance in order
 * 
 */
typedef char modtap_list_def[MAX_TAPDANCE][NVS_NS_NAME_MAX_SIZE]; 




typedef struct dd_macro_group_str 
{
	char name[MG_NAME_LENGTH];
}dd_macro_group;


/**
 * @brief 
 * 
 */
typedef struct dd_leaderkey_str {
	char name[LK_NAME_LENGTH];
	uint16_t keycode;
	uint8_t sequence[LK_MAX_SEQUENCE];	// Sequence of key pressed. in a 4x4 K14 would be 4, and K21 would be 5.
}dd_leaderkey;




typedef struct dd_macro_str {
	char name[MACRO_LONG_NAME_LENGTH];       
	char short_name[MACRO_SHORT_NAME_LENGTH];       
	uint8_t macro_group_number;
	uint16_t macro_sequence_size;
	uint8_t * macro_sequence;  			         
	// int keycode;
} dd_macro;

/**
 * @brief Structure for Macros // TODO: add new macros methodology
 * 
 */
typedef struct dd_macros_str {
	uint16_t key[MACRO_LEN];  			  //200 keys ---> MACRO_LEN = 5       
	char name[USER_MACRO_NAME_LEN];       //  200 macros ---> Name of each macro up to 32 characters
	uint16_t keycode;
} dd_macros;



extern dd_tapdance *g_user_tapdance;
extern dd_macros *g_user_macros;
extern dd_layer *g_user_layers;
extern dd_modtap *g_user_modtap;
extern dd_leaderkey *g_user_leaderkey;

extern uint8_t g_macro_num; //TODO: remove this kind of global variables
extern uint8_t g_tapdance_num;
extern uint8_t g_modtap_num;
extern uint8_t g_leaderkey_num;

#endif /* NVS_KEYMAPS_H_ */



