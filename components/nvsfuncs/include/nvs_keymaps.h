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

typedef struct dd_layer_str {
	char name[MAX_LAYOUT_NAME_LENGTH];                     // Name up to 14 characters
	uint16_t key_map[MATRIX_ROWS][MATRIX_COLS];            // Key maps
	char key_map_names[MATRIX_ROWS][MATRIX_COLS][7];       // Name of each key up to 6 characters
	uint16_t left_encoder_map[ENCODER_SIZE];               // Map ofr left encoder
	uint16_t right_encoder_map[ENCODER_SIZE];              // Map for rigth encoder
	uint16_t gesture_map[GESTURE_SIZE];                    // Map for gesture sensor
	bool active;
	char uuid_str[SHORT_UUID_STR_LEN];
} dd_layer;


#define MG_NAME_LENGTH 13

#define LK_NAME_LENGTH 13
#define LK_MAX_SEQUENCE 6

#define MT_NAME_LENGTH 13

#define TD_NAME_LENGTH 13
#define TD_MAX_TAP_SEQUENCES 6

#define MACRO_SHORT_NAME_LENGTH 7
#define MACRO_LONG_NAME_LENGTH 13

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

typedef struct dd_modtap_str {
	char name[MT_NAME_LENGTH];
	uint16_t keycode_short;			// Keycode when short pressed
	uint16_t keycode_long;			// Keycode when long pressed
}dd_modtap;

// typedef struct dd_tapdance_str {
// 	char name[TD_NAME_LENGTH];
// 	uint8_t tap_n[TD_MAX_TAP_SEQUENCES];		// Array with the number of taps to activate an action
// 												// If zero, space not activated
// 	uint16_t tap_key[TD_MAX_TAP_SEQUENCES];     // Keycode of correspnding tap dance of tap_n array
// } dd_tapdance;

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

extern dd_tapdance *user_tapdance;
extern dd_macros *user_macros;
extern dd_layer *key_layouts;

//amount of arrays
extern uint8_t layers_num;
extern uint8_t total_macros;
extern uint8_t total_tapdance;

#endif /* NVS_KEYMAPS_H_ */



