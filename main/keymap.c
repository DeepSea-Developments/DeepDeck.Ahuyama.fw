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


enum custom_macros {
	KC_CTRLALT = MACRO_BASE_VAL,  //Ubuntu windows manager (change workspace)
	KC_CTRLALTSHIFT,              //Ubuntu windows manager (move windows)
	KC_APP_SHUTTER,               //Shutter
	KC_APP_TERMINAL,             //Terminal
	KC_ALT_TAB,                  //Alt Tab
	KC_ALT_SHIFT_TAB,            //Alt Shift Tab
	KC_APP_CHROME_NTAB,            //Chrome - Next tab
	KC_APP_CHROME_PTAB,            //Chrome - Prev tab
	KC_APP_CHROME_NEW_TAB,            //Chrome - New window
	KC_APP_CHROME_NEW_WINDOW,            //Chrome - New window
	KC_APP_CHROME_N_INC_WINDOW,       //Chrome - New incognito window
	KC_APP_CHROME_CLOSE_TAB,           //Chrome - Close tab
	KC_APP_WINDOWPUT_LINUX_K		// Windows plugin call windowput to move windows to certain parts. use this plus arrows.

};

/*define what the macros do
 * important- make sure you you put the macros in the same order as the their enumeration
 */
uint16_t macros[MACROS_NUM][MACRO_LEN] = {
		// KC_CTR + ALT - //Ubuntu windows manager (change workspace)
		{ KC_LCTRL, KC_LALT, KC_NO },
		// KC_CTR + ALT + Shift - Ubuntu windows manager (move windows)
		{ KC_LCTRL, KC_LALT, KC_LSHIFT, KC_NO },
		//Shutter
		{ KC_LALT, KC_LSHIFT, KC_S, KC_NO },
		//Terminal
		{ KC_LCTRL, KC_LALT, KC_T, KC_NO}, 
		//alt + Tab
		{ KC_LALT, KC_TAB, KC_NO}, 
		//alt + Shift + Tab
		{ KC_LALT, KC_LSHIFT, KC_TAB, KC_NO}, 
		//Chrome - Next tab
		{ KC_LCTRL, KC_TAB, KC_NO}, 
		//Chrome - Prev tab
		{ KC_LCTRL, KC_LSHIFT, KC_TAB, KC_NO},
		//Chrome - New tab
		{ KC_LCTRL, KC_T, KC_NO}, 
		//Chrome - New Window
		{ KC_LCTRL, KC_N, KC_NO}, 
		//Chrome - New incognito window
		{ KC_LCTRL, KC_LSHIFT, KC_N, KC_NO}, 
		//Chrome - Close tab
		{ KC_LCTRL, KC_W, KC_NO}, 
		//Linux - WindowPut plugin - MosaicWindow
		{ KC_LGUI, KC_LALT, KC_NO}, 
	};

/*Encoder keys for each layer by order, and for each pad
 * First variable states what usage the encoder has
 */

uint16_t default_encoder_map[LAYERS][ENCODER_SIZE] = {
		/* Encoder 1 layout
			 * ,-----------------------------------------------------------------------------------------------------------.
			 * |  ---Layers---   |  CCW rotation   |    CW rotation    |      PUSH     |    LONG PUSH    |   DOUBLE PUSH   |
			 * |-----------------+-----------------+-------------------+---------------+-----------------+-----------------|
			 * |        L1   --> |    VOL UP       |    VOL DOWN       |      MUTE     |       PLAY      |    NEXT SONG    |
			 * |-----------------+-----------------+-------------------+---------------+-----------------+-----------------|
			 * |        L2   --> |    VOL UP       |    VOL DOWN       |      MUTE     |       PLAY      |    NEXT SONG    |
			 * |-----------------+-----------------+-------------------+---------------+-----------------+-----------------|
			 * |        L3   --> |    VOL UP       |    VOL DOWN       |      MUTE     |       PLAY      |    NEXT SONG    |
			 * `-----------------------------------------------------------------------------------------------------------'
			 */
		{ KC_AUDIO_VOL_DOWN, KC_AUDIO_VOL_UP, KC_AUDIO_MUTE, KC_MEDIA_PLAY_PAUSE, KC_MEDIA_NEXT_TRACK },
		{ KC_AUDIO_VOL_DOWN, KC_AUDIO_VOL_UP, KC_AUDIO_MUTE, KC_MEDIA_PLAY_PAUSE, KC_MEDIA_NEXT_TRACK },
		{ KC_AUDIO_VOL_DOWN, KC_AUDIO_VOL_UP, KC_AUDIO_MUTE, KC_MEDIA_PLAY_PAUSE, KC_MEDIA_NEXT_TRACK }
		//{ MEDIA_ENCODER, KC_AUDIO_VOL_UP, KC_AUDIO_VOL_DOWN, KC_AUDIO_MUTE },
		// |Y+|Y-| LEFT CLICK|
		// { MOUSE_ENCODER, KC_MS_UP, KC_MS_DOWN, KC_MS_BTN2 },
		// { MOUSE_ENCODER, KC_MS_UP, KC_MS_DOWN, KC_MS_BTN2 } 
	};

uint16_t default_slave_encoder_map[LAYERS][ENCODER_SIZE] = {
	/* Encoder 2 layout
			 * ,-----------------------------------------------------------------------------------------------------------.
			 * |  ---Layers---   |  CCW rotation   |    CW rotation    |      PUSH     |    LONG PUSH    |   DOUBLE PUSH   |
			 * |-----------------+-----------------+-------------------+---------------+-----------------+-----------------|
			 * |        L1   --> | CHROME-NEXT TAB | CHROME-PREV TAB   |CHROME-NEW TAB |CHROME-NEW WINDW |CHROME-NEXT INCOG|
			 * |-----------------+-----------------+-------------------+---------------+-----------------+-----------------|
			 * |        L2   --> | CHROME-NEXT TAB | CHROME-PREV TAB   |CHROME-NEW TAB |CHROME-NEW WINDW |CHROME-NEXT INCOG|
			 * |-----------------+-----------------+-------------------+---------------+-----------------+-----------------|
			 * |        L3   --> | CHROME-NEXT TAB | CHROME-PREV TAB   |CHROME-NEW TAB |CHROME-NEW WINDW |CHROME-NEXT INCOG|
			 * `-----------------------------------------------------------------------------------------------------------'
			 */
		// |VOL + | VOL - | MUTE |
		{ KC_APP_CHROME_PTAB, KC_APP_CHROME_NTAB, KC_APP_CHROME_NEW_TAB, KC_APP_CHROME_CLOSE_TAB, KC_APP_CHROME_N_INC_WINDOW },
		// |Y+|Y-| LEFT CLICK|
		{ KC_APP_CHROME_PTAB, KC_APP_CHROME_NTAB, KC_APP_CHROME_NEW_TAB, KC_APP_CHROME_CLOSE_TAB, KC_APP_CHROME_N_INC_WINDOW },
		// |Y+|Y-| LEFT CLICK|
		{ KC_APP_CHROME_PTAB, KC_APP_CHROME_NTAB, KC_APP_CHROME_NEW_TAB, KC_APP_CHROME_CLOSE_TAB, KC_APP_CHROME_N_INC_WINDOW }
	 };

// Fillers to make layering more clear
#define _______ KC_TRNS
#define XXXXXXX KC_NO

// Each keymap is represented by an array, with an array that points to all the keymaps  by order
	 uint16_t _QWERTY[MATRIX_ROWS][KEYMAP_COLS]={

			/* MEDIA
			 * ,-----------------------------------------------------------------------.
			 * |        <<       |        |>       |       >>        |  LAYER CHANGE   |
			 * |-----------------+-----------------+-----------------+-----------------|
			 * |   CHROME_PTAB   |    CHROME_NTAB  |    TERMINAL     |     SHUTTER     |
			 * |-----------------+-----------------+-----------------+-----------------|
			 * |    WINDOW_M     |  WINDOWS_KEY    |       UP        |    WINDOWPUT_K  |
			 * |-----------------+-----------------+-----------------+-----------------|
			 * |    WINDOW_W     |     LEFT        |      DOWN       |      RIGHT      |
			 * `-----------------------------------------------------------------------'
			 */

			  {KC_MPRV,             KC_MPLY,            KC_MNXT,            RAISE },
			  {KC_APP_CHROME_PTAB,  KC_APP_CHROME_NTAB, KC_APP_TERMINAL,    KC_APP_SHUTTER },
			  {KC_CTRLALTSHIFT,     KC_LGUI,            KC_UP,              KC_APP_WINDOWPUT_LINUX_K} ,
			  {KC_CTRLALT,          KC_LEFT,            KC_DOWN,            KC_RIGHT}

	};

	 uint16_t _NUM[MATRIX_ROWS][KEYMAP_COLS]={

		 	/* MEDIA
			 * ,-----------------------------------------------------------------------.
			 * |        1        |         2       |       3         |  LAYER CHANGE   |
			 * |-----------------+-----------------+-----------------+-----------------|
			 * |        4        |         5       |       6         |        +        |
			 * |-----------------+-----------------+-----------------+-----------------|
			 * |        7        |         8       |       9         |        *        |
			 * |-----------------+-----------------+-----------------+-----------------|
			 * |        .        |         0       |        <-       |      ENTER      |
			 * `-----------------------------------------------------------------------'
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

