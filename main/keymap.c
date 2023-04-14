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
	KC_APP_CHROME_TAB1,              // Chrome - Open tab 1
	KC_APP_CHROME_TAB2,              // Chrome - Open tab 2
	KC_APP_CHROME_TAB3,              // Chrome - Open tab 3
	KC_APP_CHROME_TAB_LAST,			 // Chrome - Open last tab
	KC_APP_CHROME_REOPEN_TABS,       // Chrome - Reopen all the tabs
	KC_APP_CHROME_BACKWARD,          // Chrome - Navigate Backward
	KC_APP_CHROME_FORWARD,           // Chrome - Navigate forward
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

	KC_APP_COPY,  // Copy
	KC_APP_PASTE, // Paste
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
	//Chrome - Navigate Backward
	{KC_LALT, KC_LEFT, KC_NO},
	//Chrome - Navigate forward
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

};

// Fillers to make layering more clear
#define _______ KC_TRNS
#define XXXXXXX KC_NO

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
			{KC_MPLY, KC_MNXT, KC_AUDIO_MUTE, RAISE},
			{KC_ALT_TAB, KC_ALT_SHIFT_TAB, KC_F11, KC_PSCR},
			{KC_APP_COPY, KC_APP_PASTE, KC_UP, KC_ENTER},
			{KC_LGUI, KC_LEFT, KC_DOWN, KC_RIGHT}},
		.key_map_names = {{"Play", "next", "mute", "layer"}, {"nWind", "PWind", "F11", "PrtSC"}, {"Copy", "Paste", "up", "Enter"}, {"window", "left", "down", "right"}},
		// Knobs - {CW, CCW, Single Press, Long Press, Double press}
		.left_encoder_map = {KC_AUDIO_VOL_DOWN, KC_AUDIO_VOL_UP, KC_MEDIA_PLAY_PAUSE, KC_AUDIO_MUTE, KC_MEDIA_NEXT_TRACK},
		.right_encoder_map = {KC_APP_CHROME_PTAB, KC_APP_CHROME_NTAB, KC_APP_CHROME_NEW_TAB, KC_APP_CHROME_CLOSE_TAB, KC_APP_CHROME_N_INC_WINDOW},
		// APDS9960 -  {UP, DOWN, LEFT, RIGHT, NEAR, FAR}
		.gesture_map = {KC_AUDIO_VOL_DOWN, KC_AUDIO_VOL_UP, KC_MEDIA_PLAY_PAUSE, KC_AUDIO_MUTE, KC_MEDIA_NEXT_TRACK, KC_MEDIA_NEXT_TRACK},
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
		.right_encoder_map = {KC_APP_VSCODE_FORWARD, KC_APP_VSCODE_BACKWARD, KC_APP_VSCODE_REDO, KC_APP_VSCODE_FIND_ALL, KC_APP_VSCODE_FIND},
		// APDS9960 -  {UP, DOWN, LEFT, RIGHT, NEAR, FAR}
		.gesture_map = {KC_APP_VSCODE_FORWARD, KC_APP_VSCODE_BACKWARD, KC_APP_VSCODE_REDO, KC_APP_VSCODE_FIND_ALL, KC_APP_VSCODE_FIND},
};

// dd_layer user_layer1={0};
// dd_layer user_layer2={0};

dd_layer *default_layouts[LAYERS] = {&layer1, &layer2, &layer3};

uint8_t current_layout = 0;

#endif
