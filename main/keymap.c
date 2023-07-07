#ifndef KEYMAP_C
#define KEYMAP_C

#include "key_definitions.h"
#include "keyboard_config.h"
#include "keymap.h"
#include "plugins.h"
#include "nvs_keymaps.h"
#include "stdlib.h"

// A bit different from QMK, default returns you to the first layer, LOWER and raise increase/lower layer by order.
#define DEFAULT 400 // 0x100 ->
#define LOWER 401 // 0x101
#define RAISE 402 // 0x102

#define DD_LAYER_INIT(name)                              \
	{                                                    \
		name, {{0}}, {{{0}}}, {0}, {0}, {0}, false, {0}, \
	}

// // Keymaps are designed to be relatively interchangeable with QMK
// enum custom_keycodes {
// 	QWERTY, NUM,
//     PLUGINS,
// };

// Set these for each layer and use when layers are needed in a hold-to use layer
enum layer_holds
{
	QWERTY_H = LAYER_HOLD_BASE_VAL,
	NUM_H,
	FUNCS_H
};

// // array to hold names of layouts for oled
// char default_layout_names[LAYERS][MAX_LAYOUT_NAME_LENGTH] = { "MEDIA", "NUM",
// 		  "VSCODE",
// 		};

/* select a keycode for your macro
 * important - first macro must be initialized as MACRO_BASE_VAL
 * */

enum custom_macros
{
	KC_CTRLALT = MACRO_BASE_VAL,	 // Ubuntu windows manager (change workspace)
	KC_CTRLALTSHIFT,				 // Ubuntu windows manager (move windows)
	KC_APP_SHUTTER,					 // Shutter
	KC_APP_TERMINAL,				 // Terminal
	KC_ALT_TAB,						 // Alt Tab
	KC_ALT_SHIFT_TAB,				 // Alt Shift Tab
	KC_APP_CHROME_NTAB,				 // Chrome - Next tab
	KC_APP_CHROME_PTAB,				 // Chrome - Prev tab
	KC_APP_CHROME_NEW_TAB,			 // Chrome - New window
	KC_APP_CHROME_NEW_WINDOW,		 // Chrome - New window
	KC_APP_CHROME_N_INC_WINDOW,		 // Chrome - New incognito window
	KC_APP_CHROME_CLOSE_TAB,		 // Chrome - Close tab
	KC_APP_CHROME_TAB1,				 // Chrome - Open tab 1
	KC_APP_CHROME_TAB2,				 // Chrome - Open tab 2
	KC_APP_CHROME_TAB3,				 // Chrome - Open tab 3
	KC_APP_CHROME_TAB_LAST,			 // Chrome - Open last tab
	KC_APP_CHROME_REOPEN_TABS,		 // Chrome - Reopen all the tabs
	KC_APP_CHROME_BACKWARD,			 // Chrome - Navigate Backward
	KC_APP_CHROME_FORWARD,			 // Chrome - Navigate forward
	KC_APP_WINDOWPUT_LINUX_K,		 // Windows plugin call windowput to move windows to certain parts. use this plus arrows.
	KC_APP_GIMP_DESELECT,			 // Gimp shortcut to deselect all.
	KC_APP_GIMP_INVERT,				 // Gimp shortcut to invert selection.
	KC_APP_GIMP_FIT_IMAGE,			 // Gimp shortcut to fir image to window.
	KC_APP_VSCODE_TOGGLE_SIDEBAR,	 // VSCode: Toggle sidebar.
	KC_APP_VSCODE_COMMENT_LINE,		 // VSCode: Toggle line comment.
	KC_APP_VSCODE_COMMENT_SELECTION, // VSCode: Toggle selection comment.
	KC_APP_VSCODE_MULT_SELECTION,	 // VSCode: Multiple selection
	KC_APP_VSCODE_FORWARD,			 // VSCode: Go forward
	KC_APP_VSCODE_BACKWARD,			 // VSCode: Go Backward
	KC_APP_VSCODE_FIND,				 // VSCode: Find
	KC_APP_VSCODE_FIND_ALL,			 // VSCode: Find all
	KC_APP_VSCODE_UNDO,				 // VSCode: undo
	KC_APP_VSCODE_REDO,				 // VSCode: redo

	KC_APP_COPY,	 // Copy
	KC_APP_PASTE,	 // Paste
	KC_SPECIAL_LINK, // SPECIAL LINK
};

/*define what the macros do
 * important- make sure you you put the macros in the same order as the their enumeration
 */
uint16_t macros[MACROS_NUM][MACRO_LEN] = {
	// KC_CTR + ALT - //Ubuntu windows manager (change workspace)
	{KC_LCTRL, KC_LALT, KC_NO},
	// KC_CTR + ALT + Shift - Ubuntu windows manager (move windows)
	{KC_LCTRL, KC_LALT, KC_LSHIFT, KC_NO},
	// Shutter
	{KC_LALT, KC_LSHIFT, KC_S, KC_NO},
	// Terminal
	{KC_LCTRL, KC_LALT, KC_T, KC_NO},
	// alt + Tab
	{KC_LALT, KC_TAB, KC_NO},
	// alt + Shift + Tab
	{KC_LALT, KC_LSHIFT, KC_TAB, KC_NO},

	// Chrome - Next tab
	{KC_LCTRL, KC_TAB, KC_NO},
	// Chrome - Prev tab
	{KC_LCTRL, KC_LSHIFT, KC_TAB, KC_NO},
	// Chrome - New tab
	{KC_LCTRL, KC_T, KC_NO},
	// Chrome - New Window
	{KC_LCTRL, KC_N, KC_NO},
	// Chrome - New incognito window
	{KC_LCTRL, KC_LSHIFT, KC_N, KC_NO},
	// Chrome - Close tab
	{KC_LCTRL, KC_W, KC_NO},
	//  Chrome - Open tab 1
	{KC_LCTRL, KC_1, KC_NO},
	//  Chrome - Open tab 2
	{KC_LCTRL, KC_2, KC_NO},
	//  Chrome - Open tab 3
	{KC_LCTRL, KC_3, KC_NO},
	//  Chrome - Open last tab
	{KC_LCTRL, KC_9, KC_NO},
	//  Chrome - Reopen all the tabs
	{KC_LCTRL, KC_LSHIFT, KC_T, KC_NO},
	// Chrome - Navigate Backward
	{KC_LALT, KC_LEFT, KC_NO},
	// Chrome - Navigate forward
	{KC_LALT, KC_RIGHT, KC_NO},

	// Linux - WindowPut plugin - MosaicWindow
	{KC_LGUI, KC_LALT, KC_NO},

	// GIMP - Select none
	{KC_LCTRL, KC_LSHIFT, KC_A, KC_NO},
	// GIMP - Invert Selection
	{KC_LCTRL, KC_I, KC_NO},
	// GIMP - Fit image
	{KC_LCTRL, KC_LSHIFT, KC_J, KC_NO},

	// VSCode: Toggle sidebar
	{KC_LCTRL, KC_B, KC_NO},
	// VSCode: Toggle line comment.
	{KC_LCTRL, KC_SLASH, KC_NO},
	// VSCode: Toggle selection comment.
	{KC_LCTRL, KC_LSHIFT, KC_A, KC_NO},
	// VSCode: Multiple selection
	{KC_LCTRL, KC_D, KC_NO},
	// VSCode: Go forward
	{KC_LCTRL, KC_LALT, KC_MINUS, KC_NO},
	// VSCode: Go Backward
	{KC_LCTRL, KC_LSHIFT, KC_MINUS, KC_NO},
	// VSCode: Find
	{KC_LCTRL, KC_F, KC_NO},
	// VSCode: Find all
	{KC_LCTRL, KC_LSHIFT, KC_F, KC_NO},

	// VSCode: undo
	{KC_LCTRL, KC_Z, KC_NO},
	// VSCode: redo
	{KC_LCTRL, KC_Y, KC_NO},

	//  Copy
	{KC_LCTRL, KC_C, KC_NO},
	//  Paste
	{KC_LCTRL, KC_V, KC_NO},

	{KC_LCTRL, KC_K, KC_NO},

};

// char *default_macro_name[] = {"KC_CTRLALT", "KC_CTRLALTSHIFT", "KC_APP_SHUTTER", "KC_APP_TERMINAL", "KC_ALT_TAB",
// 							  "KC_ALT_SHIFT_TAB", "KC_APP_CHROME_NTAB", "KC_APP_CHROME_PTAB", "KC_APP_CHROME_NEW_TAB",
// 							  "KC_APP_CHROME_NEW_WINDOW", "KC_APP_CHROME_N_INC_WINDOW", "KC_APP_CHROME_CLOSE_TAB",
// 							  "KC_APP_CHROME_TAB1", "KC_APP_CHROME_TAB2", "KC_APP_CHROME_TAB3", "KC_APP_CHROME_TAB_LAST",
// 							  "KC_APP_CHROME_REOPEN_TABS", "KC_APP_CHROME_BACKWARD", "KC_APP_CHROME_FORWARD",
// 							  "KC_APP_WINDOWPUT_LINUX_K", "KC_APP_GIMP_DESELECT", "KC_APP_GIMP_INVERT", "KC_APP_GIMP_FIT_IMAGE",
// 							  "KC_APP_VSCODE_TOGGLE_SIDEBAR", "KC_APP_VSCODE_COMMENT_LINE", "KC_APP_VSCODE_COMMENT_SELECTION",
// 							  "KC_APP_VSCODE_MULT_SELECTION", "KC_APP_VSCODE_FORWARD", "KC_APP_VSCODE_BACKWARD", "KC_APP_VSCODE_FIND",
// 							  "KC_APP_VSCODE_FIND_ALL", "KC_APP_VSCODE_UNDO", "KC_APP_VSCODE_REDO", "KC_APP_COPY", "KC_APP_PASTE",
// 							  "KC_SPECIAL_LINK"};

char *default_macro_name[] = {"macro_1","macro_2","macro_3","macro_4","macro_5","macro_6","macro_7","macro_8","macro_9",
							  "macro_10","macro_11","macro_12","macro_13","macro_14","macro_15","macro_16","macro_17","macro_18","macro_19",
							  "macro_20","macro_21","macro_22","macro_23","macro_24","macro_25","macro_26","macro_27","macro_28","macro_29",
							  "macro_30","macro_31","macro_32","macro_33","macro_34","macro_35","macro_36","macro_37","macro_38","macro_39",
							  "macro_40","macro_41","macro_42","macro_43","macro_44","macro_45","macro_46","macro_47","macro_48","macro_49",
							  "macro_50","macro_51","macro_52","macro_53","macro_54","macro_55","macro_56","macro_57","macro_58","macro_59",
							  "macro_60","macro_61","macro_62","macro_63","macro_64","macro_65","macro_66","macro_67","macro_68","macro_69",
							  "macro_70","macro_71","macro_72","macro_73","macro_74","macro_75","macro_76","macro_77","macro_78","macro_79",
							  "macro_80","macro_81","macro_82","macro_83","macro_84","macro_85","macro_86","macro_87","macro_88","macro_89",
							  "macro_90"};

// Fillers to make layering more clear
#define _______ KC_TRNS
#define XXXXXXX KC_NO
char encoder_items_names[ENCODER_SIZE][15] = {"cw", "ccw", "single_press", "long_press", "double_press"};
char gesture_items_names[GESTURE_SIZE][8] = {"up", "down", "left", "right", "near", "far"};

dd_layer layer1 =
	{
		.name = "Media",
		/* MEDIA
		 * ,-----------------------------------------------------------------------.
		 * |     Play/5Pause  |     Next song   |       Mute      |  LAYER CHANGE   |
		 * |-----------------+-----------------+-----------------+-----------------|
		 * |    next window  | Prev window     | full screen(f11)|  Print Screen   |
		 * |-----------------+-----------------+-----------------+-----------------|
		 * |       Copy      |      Paste      |       UP        |       Enter     |
		 * |-----------------+-----------------+-----------------+-----------------|
		 * |    WINDOW_W     |     LEFT        |      DOWN       |      RIGHT      |
		 * `-----------------------------------------------------------------------'
		 */
		.key_map = {
			{KC_MPLY, KC_MNXT, KC_AUDIO_MUTE, RAISE},		 // row0
			{KC_ALT_TAB, KC_ALT_SHIFT_TAB, KC_F11, KC_PSCR}, // row1
			{KC_APP_COPY, KC_APP_PASTE, KC_UP, KC_ENTER},
			{KC_LGUI, KC_LEFT, KC_DOWN, KC_RIGHT}},
		.key_map_names = {{"Play", "next", "mute", "layer"}, {"nWind", "PWind", "F11", "PrtSC"}, {"Copy", "Paste", "up", "Enter"}, {"window", "left", "down", "right"}},
		// Knobs - {CW, CCW, Single Press, Long Press, Double press}
		.left_encoder_map = {KC_AUDIO_VOL_DOWN, KC_AUDIO_VOL_UP, KC_MEDIA_PLAY_PAUSE, KC_AUDIO_MUTE, KC_MEDIA_NEXT_TRACK},
		.right_encoder_map = {KC_APP_CHROME_PTAB, KC_APP_CHROME_NTAB, KC_APP_CHROME_NEW_TAB, KC_APP_CHROME_CLOSE_TAB, KC_APP_CHROME_N_INC_WINDOW},
		// APDS9960 -  {UP, DOWN, LEFT, RIGHT, NEAR, FAR}
		.gesture_map = {KC_AUDIO_VOL_DOWN, KC_AUDIO_VOL_UP, KC_MEDIA_PLAY_PAUSE, KC_AUDIO_MUTE, KC_MEDIA_NEXT_TRACK, KC_MEDIA_NEXT_TRACK},
		.active = true,

		// .layer_id

};

dd_layer layer2 =
	{
		.name = "Numpad",
		/* Num
		 * ,-----------------------------------------------------------------------.
		 * |        7        |         8       |       9         |  LAYER CHANGE   |
		 * |-----------------+-----------------+-----------------+-----------------|
		 * |        4        |         5       |       6         |        /        |
		 * |-----------------+-----------------+-----------------+-----------------|
		 * |        1        |         2       |       3         |        *        |
		 * |-----------------+-----------------+-----------------+-----------------|
		 * |        .        |         0       |        -        |        +        |
		 * `-----------------------------------------------------------------------'
		 */
		.key_map = {
			{KC_7, KC_8, KC_9, RAISE},
			{KC_4, KC_5, KC_6, KC_KP_SLASH},
			{KC_1, KC_2, KC_3, KC_KP_ASTERISK},
			{KC_DOT, KC_0, KC_KP_MINUS, KC_KP_PLUS}},
		.key_map_names = {{"7", "8", "9", "layer"}, {"4", "5", "6", "/"}, {"1", "2", "3", "*"}, {".", "0", "-", "+"}},
		// Knobs - {CW, CCW, Single Press, Long Press, Double press}
		.left_encoder_map = {KC_LEFT, KC_RIGHT, KC_APP_COPY, KC_APP_PASTE, KC_APP_PASTE},
		.right_encoder_map = {KC_DOWN, KC_UP, KC_ENTER, KC_TAB, KC_TAB},
		// APDS9960 -  {UP, DOWN, LEFT, RIGHT, NEAR, FAR}
		.gesture_map = {KC_0, KC_1, KC_2, KC_3, KC_4, KC_5},
		.active = true,
};

dd_layer layer3 =
	{
		.name = "Chrome",
		/* Chrome
		 * ,-----------------------------------------------------------------------.
		 * | F12 (dev tools) | F11 (large screen) |     PrtScr   |  LAYER CHANGE   |
		 * |-----------------+-----------------+-----------------+-----------------|
		 * |   Next TAB      |  Previous TAB   |    Reopen tabs  |   close tab     |
		 * |-----------------+-----------------+-----------------+-----------------|
		 * |  Navigate back  |Navigate forward |   New Tab       |New incognito tab|
		 * |-----------------+-----------------+-----------------+-----------------|
		 * |       Tab 1     |      Tab 2      |     Tab 3       |      Tab n      |
		 * `-----------------------------------------------------------------------'
		 */
		.key_map = {
			{KC_F12, KC_F11, KC_PSCREEN, RAISE},
			{KC_APP_CHROME_PTAB, KC_APP_CHROME_NTAB, KC_APP_CHROME_REOPEN_TABS, KC_APP_CHROME_CLOSE_TAB},
			{KC_APP_CHROME_BACKWARD, KC_APP_CHROME_FORWARD, KC_APP_CHROME_NEW_TAB, KC_APP_CHROME_N_INC_WINDOW},
			{KC_APP_CHROME_TAB1, KC_APP_CHROME_TAB2, KC_APP_CHROME_TAB3, KC_APP_CHROME_TAB_LAST}},
		.key_map_names = {{"dev", "fullscr", "prtscr", "layer"}, {"PTAB", "NTAB", "reopne", "close"}, {"back", "forw", "new", "incog"}, {"Tab1", "Tab2", "Tab3", "TabN"}},

		// Knobs - {CW, CCW, Single Press, Long Press, Double press}
		.left_encoder_map = {KC_AUDIO_VOL_DOWN, KC_AUDIO_VOL_UP, KC_APP_VSCODE_UNDO, KC_MEDIA_PLAY_PAUSE, KC_MEDIA_NEXT_TRACK},
		.right_encoder_map = {KC_APP_CHROME_FORWARD, KC_APP_CHROME_BACKWARD, KC_APP_CHROME_NEW_TAB, KC_APP_CHROME_NEW_WINDOW, KC_APP_CHROME_CLOSE_TAB},
		// APDS9960 -  {UP, DOWN, LEFT, RIGHT, NEAR, FAR}
		.gesture_map = {KC_APP_CHROME_CLOSE_TAB, 0, KC_APP_CHROME_NTAB, KC_APP_CHROME_PTAB, 0, 0},
		.active = true,
};

// dd_layer user_layer[3] = {
// 	DD_LAYER_INIT("USER1"),
// 	DD_LAYER_INIT("USER2"),
// 	DD_LAYER_INIT("USER3"),
// };

dd_layer *default_layouts[LAYERS] = {&layer1, &layer2, &layer3};
uint8_t current_layout = 0;

dd_macros default_macros[USER_MACROS_NUM]={0};
dd_macros *ptr_default_macros [USER_MACROS_NUM];; 

void generate_uuid()
{
	uuid_t uu;
	char uu_str[SHORT_UUID_STR_LEN];

	for (int i = 0; i < LAYERS; i++)
	{
		uuid_generate(uu);
		short_uuid_unparse(uu, uu_str);
		strcpy(default_layouts[i]->uuid_str, uu_str);
	}
}

void init_default_macros()
{
	int i = 0;
	int j = 0;
	for (i = 0; i < MACROS_NUM; i++)
	{
		ptr_default_macros[i] = &default_macros[i];
		for(j= 0; j < (MACRO_LEN-1); j++){
		default_macros[i].key[j] = macros[i][j];
		}
		strcpy(default_macros[i].name, default_macro_name[i]);
		default_macros[i].keycode= MACRO_BASE_VAL+i;	
	}

}

#endif
