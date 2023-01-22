#ifndef KEYMAP_C
#define KEYMAP_C

#include "key_definitions.h"
#include "keyboard_config.h"
#include "keymap.h"
#include "plugins.h"
#include "nvs_keymaps.h"

// A bit different from QMK, default returns you to the first layer, LOWER and raise increase/lower layer by order.
#define DEFAULT 0x100
#define LOWER 0x101
#define RAISE 0x102

// // Keymaps are designed to be relatively interchangeable with QMK
// enum custom_keycodes {
// 	QWERTY, NUM,
//     PLUGINS,
// };

//Set these for each layer and use when layers are needed in a hold-to use layer
enum layer_holds {
	QWERTY_H = LAYER_HOLD_BASE_VAL, NUM_H,FUNCS_H
};

// // array to hold names of layouts for oled
// char default_layout_names[LAYERS][MAX_LAYOUT_NAME_LENGTH] = { "MEDIA", "NUM",
// 		  "VSCODE",
// 		};

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
	KC_APP_WINDOWPUT_LINUX_K,		// Windows plugin call windowput to move windows to certain parts. use this plus arrows.
	KC_APP_GIMP_DESELECT,		// Gimp shortcut to deselect all.
	KC_APP_GIMP_INVERT,		// Gimp shortcut to invert selection.
	KC_APP_GIMP_FIT_IMAGE,		// Gimp shortcut to fir image to window.
	KC_APP_VSCODE_TOGGLE_SIDEBAR,		// VSCode: Toggle sidebar.
	KC_APP_VSCODE_COMMENT_LINE,		// VSCode: Toggle line comment.
	KC_APP_VSCODE_COMMENT_SELECTION,		// VSCode: Toggle selection comment.
	KC_APP_VSCODE_MULT_SELECTION,		// VSCode: Multiple selection
	KC_APP_VSCODE_FORWARD,		// VSCode: Go forward
	KC_APP_VSCODE_BACKWARD,		// VSCode: Go Backward
	KC_APP_VSCODE_FIND,		// VSCode: Find
	KC_APP_VSCODE_FIND_ALL,		// VSCode: Find all
	KC_APP_VSCODE_UNDO,		// VSCode: undo
	KC_APP_VSCODE_REDO,		// VSCode: redo

	KC_APP_COPY, //Copy
	KC_APP_PASTE, //Paste
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

		// GIMP - Select none
		{ KC_LCTRL, KC_LSHIFT, KC_A ,KC_NO}, 
		// GIMP - Invert Selection
		{ KC_LCTRL, KC_I ,KC_NO},
		// GIMP - Fit image
		{ KC_LCTRL, KC_LSHIFT, KC_J ,KC_NO}, 

		// VSCode: Toggle sidebar
		{ KC_LCTRL, KC_B ,KC_NO}, 
		// VSCode: Toggle line comment.
		{ KC_LCTRL, KC_SLASH ,KC_NO},
		// VSCode: Toggle selection comment.
		{ KC_LCTRL, KC_LSHIFT, KC_A ,KC_NO}, 
		// VSCode: Multiple selection
		{ KC_LCTRL, KC_D ,KC_NO}, 
		// VSCode: Go forward
		{ KC_LCTRL, KC_LALT, KC_MINUS ,KC_NO}, 
		// VSCode: Go Backward
		{ KC_LCTRL, KC_LSHIFT, KC_MINUS ,KC_NO},
		// VSCode: Find
		{ KC_LCTRL, KC_F ,KC_NO}, 
		// VSCode: Find all
		{ KC_LCTRL, KC_LSHIFT, KC_F ,KC_NO}, 
		
		// VSCode: undo
		{ KC_LCTRL, KC_Z ,KC_NO}, 
		// VSCode: redo
		{ KC_LCTRL, KC_Y ,KC_NO}, 

		//  Copy
		{ KC_LCTRL, KC_C ,KC_NO}, 
		//  Paste
		{ KC_LCTRL, KC_V ,KC_NO}, 

	};
	 

// Fillers to make layering more clear
#define _______ KC_TRNS
#define XXXXXXX KC_NO


dd_layer layer1 = 
		{
			.name = "Media",
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
			.key_map = {
					{KC_MPRV,             KC_MPLY,            KC_MNXT,            RAISE },
					{KC_APP_CHROME_PTAB,  KC_APP_CHROME_NTAB, KC_APP_TERMINAL,    KC_APP_SHUTTER },
					{KC_CTRLALTSHIFT,     KC_LGUI,            KC_UP,              KC_APP_WINDOWPUT_LINUX_K} ,
					{KC_CTRLALT,          KC_LEFT,            KC_DOWN,            KC_RIGHT}
			},
			.key_map_names = {
				{"prev",  "play",  "next", "layer"},
				{"PTAB",  "NTAB",  "Term", "Shutt"},
				{"copy",  "GUI",  "up", "RAISE"},
				{"copy",  "left",  "down", "right"}
			},
			.left_encoder_map = 
				{KC_AUDIO_VOL_DOWN, KC_AUDIO_VOL_UP, KC_MEDIA_PLAY_PAUSE, KC_AUDIO_MUTE,  KC_MEDIA_NEXT_TRACK },
			.right_encoder_map = 
				{ KC_APP_CHROME_PTAB, KC_APP_CHROME_NTAB, KC_APP_CHROME_NEW_TAB, KC_APP_CHROME_CLOSE_TAB, KC_APP_CHROME_N_INC_WINDOW },
			.gesture_map = 
				{ KC_AUDIO_VOL_DOWN, KC_AUDIO_VOL_UP, KC_MEDIA_PLAY_PAUSE, KC_AUDIO_MUTE,  KC_MEDIA_NEXT_TRACK,KC_MEDIA_NEXT_TRACK },
		};

dd_layer layer2 = 
		{
			.name = "Num",
			/* Num
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
			.key_map = {
					{KC_1,       KC_2,          KC_3,    RAISE  },
					{KC_4,       KC_5,          KC_6,    KC_KP_PLUS  },
					{KC_7,       KC_8,          KC_9,    KC_KP_ASTERISK },
					{KC_DOT,     KC_0,     KC_BSPACE,    KC_ENTER }
			},
			.key_map_names = {
				{"1",  "2",  "3", "layer"},
				{"4",  "5",  "6", "+"},
				{"7",  "8",  "9", "*"},
				{".",  "0",  "<-", "enter"}
			},
			.left_encoder_map = 
				{KC_AUDIO_VOL_DOWN, KC_AUDIO_VOL_UP, KC_MEDIA_PLAY_PAUSE, KC_AUDIO_MUTE,  KC_MEDIA_NEXT_TRACK },
			.right_encoder_map = 
				{ KC_APP_CHROME_PTAB, KC_APP_CHROME_NTAB, KC_APP_CHROME_NEW_TAB, KC_APP_CHROME_CLOSE_TAB, KC_APP_CHROME_N_INC_WINDOW },
			.gesture_map = 
				{ KC_AUDIO_VOL_DOWN, KC_AUDIO_VOL_UP, KC_MEDIA_PLAY_PAUSE, KC_AUDIO_MUTE,  KC_MEDIA_NEXT_TRACK,KC_MEDIA_NEXT_TRACK },
		};

dd_layer layer3 = 
		{
			.name = "VScode",
			/* VSCODE
			 * ,-----------------------------------------------------------------------.
			 * |      COPY       |      PASTE      |       SIDEBAR   |  LAYER CHANGE   |
			 * |-----------------+-----------------+-----------------+-----------------|
			 * |    COMMENT      |    S_COMMENT    |     PALLET      |     M_SELECT    |
			 * |-----------------+-----------------+-----------------+-----------------|
			 * |       ALT       |       HOME      |       UP        |       END       |
			 * |-----------------+-----------------+-----------------+-----------------|
			 * |       CTRL      |     LEFT        |      DOWN       |      RIGHT      |
			 * `-----------------------------------------------------------------------'
			 */
			.key_map = {
					{KC_APP_COPY,  KC_APP_PASTE,    KC_APP_VSCODE_TOGGLE_SIDEBAR,    RAISE },
					{KC_APP_VSCODE_COMMENT_LINE,  KC_APP_VSCODE_COMMENT_SELECTION,    KC_F1,    KC_APP_VSCODE_MULT_SELECTION},
					{KC_LALT,     KC_HOME,            KC_UP,              KC_END} ,
					{KC_LCTRL,          KC_LEFT,            KC_DOWN,            KC_RIGHT}			
				},
			.key_map_names = {
				{"prev",  "play",  "next", "layer"},
				{"PTAB",  "NTAB",  "Term", "Shutt"},
				{"copy",  "GUI",  "up", "RAISE"},
				{"copy",  "left",  "down", "right"}
			},
			.left_encoder_map = 
				{KC_AUDIO_VOL_DOWN, KC_AUDIO_VOL_UP, KC_APP_VSCODE_UNDO, KC_MEDIA_PLAY_PAUSE,  KC_MEDIA_NEXT_TRACK },
			.right_encoder_map = 
				{ KC_APP_VSCODE_FORWARD, KC_APP_VSCODE_BACKWARD, KC_APP_VSCODE_REDO, KC_APP_VSCODE_FIND_ALL, KC_APP_VSCODE_FIND },
			.gesture_map = 
				{ KC_APP_VSCODE_FORWARD, KC_APP_VSCODE_BACKWARD, KC_APP_VSCODE_REDO, KC_APP_VSCODE_FIND_ALL, KC_APP_VSCODE_FIND },
		};


dd_layer *default_layouts[LAYERS] = {&layer1, &layer2, &layer3};


uint8_t current_layout = 0;

#endif

