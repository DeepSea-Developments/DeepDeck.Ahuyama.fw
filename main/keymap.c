#ifndef KEYMAP_C
#define KEYMAP_C

#include "key_definitions.h"
#include "keyboard_config.h"
#include "keymap.h"
#include "plugins.h"

// A bit different from QMK, default returns you to the first layer, LOWER and raise increase/lower layer by order.
#define DEFAULT 0x100
#define LOWER 0x101
#define RAISE 0x102

// Keymaps are designed to be relatively interchangeable with QMK
enum custom_keycodes {
	QWERTY, NUM,
    PLUGINS,
};

//Set these for each layer and use when layers are needed in a hold-to use layer
enum layer_holds {
	QWERTY_H = LAYER_HOLD_BASE_VAL, NUM_H,FUNCS_H
};

// array to hold names of layouts for oled
char default_layout_names[LAYERS][MAX_LAYOUT_NAME_LENGTH] = { "MEDIA", "NUM",
		  "Plugins",
		};

/* select a keycode for your macro
 * important - first macro must be initialized as MACRO_BASE_VAL
 * */

#define MACROS_NUM 20
enum custom_macros {
	KC_MACRO_COPY = MACRO_BASE_VAL, 
	KC_MACRO_PASTE,
	KC_MACRO_PASTEasd
};

/*define what the macros do
 * important- make sure you you put the macros in the same order as the their enumeration
 */
uint16_t macros[MACROS_NUM][MACRO_LEN] = {
		// KC_COPY
		{ KC_A, KC_B, KC_C, KC_D, KC_E },
		{ KC_A, KC_B, KC_NO },
		//KC_PASTE
		{ KC_LSHIFT, KC_A, KC_NO },
		{ KC_LCTRL, KC_LALT, KC_MINS }, };

/*Encoder keys for each layer by order, and for each pad
 * First variable states what usage the encoder has
 */

uint16_t default_encoder_map[LAYERS][ENCODER_SIZE] = {
		// | CCW rotation | CW rotation | PUSH | LONG PUSH | DOUBLE PUSH |
		// |VOL + | VOL - | MUTE | 
		{ KC_MS_UP, KC_MS_DOWN, KC_MS_BTN1, KC_MEDIA_PLAY_PAUSE, KC_MEDIA_NEXT_TRACK },
		//{ KC_AUDIO_VOL_DOWN, KC_AUDIO_VOL_UP, KC_AUDIO_MUTE, KC_MEDIA_PLAY_PAUSE, KC_MEDIA_NEXT_TRACK },
		//{ MEDIA_ENCODER, KC_AUDIO_VOL_UP, KC_AUDIO_VOL_DOWN, KC_AUDIO_MUTE },
		// |Y+|Y-| LEFT CLICK|
		{ MOUSE_ENCODER, KC_MS_UP, KC_MS_DOWN, KC_MS_BTN2 },

		{ MOUSE_ENCODER, KC_MS_UP, KC_MS_DOWN, KC_MS_BTN2 } };

uint16_t default_encoder2_map[LAYERS][ENCODER_SIZE] = {
		// | CCW rotation | CW rotation | PUSH | LONG PUSH | DOUBLE PUSH |
		// |VOL + | VOL - | MUTE | 
		{ KC_MS_RIGHT, KC_MS_LEFT, KC_MS_BTN2, KC_MEDIA_PLAY_PAUSE, KC_MEDIA_NEXT_TRACK },
		//{ KC_AUDIO_VOL_DOWN, KC_AUDIO_VOL_UP, KC_AUDIO_MUTE, KC_MEDIA_PLAY_PAUSE, KC_MEDIA_NEXT_TRACK },
		//{ MEDIA_ENCODER, KC_AUDIO_VOL_UP, KC_AUDIO_VOL_DOWN, KC_AUDIO_MUTE },
		// |Y+|Y-| LEFT CLICK|
		{ MOUSE_ENCODER, KC_MS_UP, KC_MS_DOWN, KC_MS_BTN2 },

		{ MOUSE_ENCODER, KC_MS_UP, KC_MS_DOWN, KC_MS_BTN2 } };

uint16_t default_slave_encoder_map[LAYERS][ENCODER_SIZE] = {
		// |VOL + | VOL - | MUTE |
		{ KC_MS_RIGHT, KC_MS_LEFT, KC_MS_BTN2, KC_MEDIA_PLAY_PAUSE, KC_MEDIA_NEXT_TRACK },
		// |Y+|Y-| LEFT CLICK|
		{ MOUSE_ENCODER, KC_MS_RIGHT, KC_MS_LEFT, KC_MS_BTN2 },
		// |Y+|Y-| LEFT CLICK|
		{ MOUSE_ENCODER, KC_MS_RIGHT, KC_MS_LEFT, KC_MS_BTN2 } };

// Fillers to make layering more clear
#define _______ KC_TRNS
#define XXXXXXX KC_NO

// Each keymap is represented by an array, with an array that points to all the keymaps  by order
	 uint16_t _QWERTY[MATRIX_ROWS][KEYMAP_COLS]={

			/* MEDIA
			 * ,----------------------------.
			 * |CALC  |  <<  |  <<  |  |>   |
			 * |------+------+------+-------|
			 * | Tab  |   A  |   S  |   D   |
			 * |------+------+------+-------|
			 * | Shift|   Z  |   X  |   C   |
			 * |------+------+------+-------|
			 * | Ctrl | GUI  |  Alt |Default|
			 * `----------------------------'
			 */

			  {KC_MPRV,  KC_MPLY,    KC_MNXT,    RAISE },
			  {KC_N,  KC_I,    KC_C,    KC_K },
			  {KC_MACRO_PASTE,KC_MACRO_COPY, KC_MACRO_PASTEasd,    KC_D} ,
			  {KC_MACRO_PASTE,KC_MACRO_COPY, KC_MACRO_PASTEasd, KC_4}

	};

	 uint16_t _NUM[MATRIX_ROWS][KEYMAP_COLS]={

		 	/* Nums
			 * ,------------------------------------------.
			 * |   1  |   2  |   3  |  |>   |  >>  |RAISE |
			 * |------+------+------+-------+------+------|
			 * |   4  |   5  |   6  |   +   |   -  |LOWER |
			 * |------+------+------+-------+------+------|
			 * |   7  |   8  |   9  |   *   |   /  | Vol+ |
			 * |------+------+------+-------+------+------|
			 * |   .  |   0  |  <-- | Enter |  GUI | Vol- |
			 * `------------------------------------------'
			 */

			  {KC_1,       KC_2,          KC_3,    RAISE  },
			  {KC_4,       KC_5,          KC_6,    KC_KP_PLUS  },
			  {KC_7,       KC_8,          KC_9,    KC_KP_ASTERISK },
			  {KC_DOT,     KC_0,     KC_BSPACE,    KC_ENTER }

	};

	 uint16_t _PLUGINS[MATRIX_ROWS][KEYMAP_COLS]={

				/* PLUGINS
				 * -----------------------------------------------------------------------------------------'
				 */

				//   {PN_CLOSE,PN_LAYOUT,PN_TEST,    KC_E,    KC_R,    KC_T },
				//   {KC_TAB,  KC_A,    KC_S,    KC_D,    KC_F,    KC_G },
				//   {KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B } ,
				//   {KC_LCTRL,KC_LGUI, KC_LALT, DEFAULT, NUM_H,   KC_SPC }
				{KC_CALC,  KC_Q,    KC_MPRV,    RAISE },
			  {KC_TAB,  KC_A,    KC_S,    KC_D},
			  {KC_LSFT, KC_Z,    KC_X,    KC_FN0 } ,
			  {KC_LCTRL,KC_LGUI, KC_LALT, DEFAULT }

		};
 //Create an array that points to the various keymaps
uint16_t (*default_layouts[])[MATRIX_ROWS][KEYMAP_COLS] = { &_QWERTY, &_NUM,
			&_PLUGINS
		};

uint8_t current_layout = 0;

#endif

