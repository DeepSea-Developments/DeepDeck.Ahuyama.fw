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


// Set these for each layer and use when layers are needed in a hold-to use layer
enum layer_holds
{
	QWERTY_H = LAYER_HOLD_BASE_VAL,
	NUM_H,
	FUNCS_H
};


enum custom_tapdance
{
	KC_TD1 = TAPDANCE_BASE_VAL,
	KC_TD2,
	KC_TD3
};

enum custom_modtap
{
	KC_MT_1 = MODTAP_BASE_VAL,
	KC_MT_2,
	KC_MT_3
};

/* select a keycode for your macro
 * important - first macro must be initialized as MACRO_BASE_VAL
 * */

enum custom_macros
{
	KC_APP_COPY = MACRO_BASE_VAL,	 // Copy
	KC_APP_PASTE,					 // Paste
	KC_ALT_TAB,	 					 // Alt Tab				 
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
	KC_M18,
	KC_M19,
	KC_M20,
	KC_M21,
	KC_M22,
	KC_M23,
	KC_M24,
	KC_M25,
	KC_M26,
	KC_M27,
	KC_M28,
	KC_M29,
	KC_M30,
	KC_M31,
	KC_M32,
	KC_M33,
	KC_M34,
	KC_M35,
	KC_M36,
	KC_M37,
	KC_M38,
	KC_M39,
	KC_M40,
};

/*define what the macros do
 * important- make sure you you put the macros in the same order as the their enumeration
 */
uint16_t macros[MACROS_NUM][MACRO_LEN] = {
	

	//  Copy
	{KC_LCTRL, KC_C, KC_NO},
	//  Paste
	{KC_LCTRL, KC_V, KC_NO},

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

	// M18
	{KC_NO},
	// M19
	{KC_NO},
	// M20
	{KC_NO},
	// M21
	{KC_NO},
	// M22
	{KC_NO},
	// M23
	{KC_NO},
	// M24
	{KC_NO},
	// M25
	{KC_NO},
	// M26
	{KC_NO},
	// M27
	{KC_NO},
	// M28
	{KC_NO},
	// M29
	{KC_NO},
	// M30
	{KC_NO},
	// M31
	{KC_NO},
	// M32
	{KC_NO},
	// M33
	{KC_NO},
	// M34
	{KC_NO},
	// M35
	{KC_NO},
	// M36
	{KC_NO},
	// M37
	{KC_NO},
	// M38
	{KC_NO},
	// M39
	{KC_NO},
	// M40
	{KC_NO},
};

char *default_macro_name[] = {	"Copy", 
								"Paste", 
								"AltTab",
								"AtlSTab", 
								"NTAB", 
								"PTAB", 
								"NEW_TAB",
								"NEW_WIND", 
								"INC_WIND", 
								"CloseTab",
								"Tab1", 
								"Tab2", 
								"Tab3", 
								"TabN",
								"ROpenT", 
								"BACKWARD", 
								"FORWARD", 
								"M18",
								"M19",
								"M20",
								"M21",
								"M22",
								"M23",
								"M24",
								"M25",
								"M26",
								"M27",
								"M28",
								"M29",
								"M30",
								"M31",
								"M32",
								"M33",
								"M34",
								"M35",
								"M36",
								"M37",
								"M38",
								"M39",
								"M40", 
								};

dd_tapdance default_tapdance[TAPDANCE_NUM]={
	{ .name="tap1",
	  .tap_list={1,2,3,10,15},
	  .keycode_list = {KC_1, KC_2, KC_AUDIO_MUTE, KC_3, KC_A},
	  .keycode = KC_TD1},
	
	{ .name="td_num",
	  .tap_list={1,2,3,4,5},
	  .keycode_list = {KC_1, KC_2, KC_3, KC_4, KC_5},
	  .keycode = KC_TD2},
	
	{ .name="td_med",
	  .tap_list={1,2,3,4,0},
	  .keycode_list = {KC_MEDIA_PLAY_PAUSE, KC_MEDIA_NEXT_TRACK, KC_MEDIA_PREV_TRACK, KC_MUTE},
	  .keycode = KC_TD3},
};


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
			{KC_TD3, KC_TD2, KC_TD1, RAISE},
			{KC_4, KC_5, KC_6, KC_KP_SLASH},
			{KC_1, KC_2, KC_3, KC_KP_ASTERISK},
			{KC_TD1, KC_TD2, KC_TD3, KC_LK}},
		.key_map_names = {{"7", "8", "9", "layer"}, {"4", "5", "6", "/"}, {"1", "2", "3", "*"}, {"TD_1", "TD_2", "TD_3", "LK"}},
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
		.key_map_names = {{"dev", "fulscr", "prtscr", "layer"}, {"PTAB", "NTAB", "reopne", "close"}, {"back", "forw", "new", "incog"}, {"Tab1", "Tab2", "Tab3", "TabN"}},

		// Knobs - {CW, CCW, Single Press, Long Press, Double press}
		.left_encoder_map = {KC_AUDIO_VOL_DOWN, KC_AUDIO_VOL_UP, KC_MEDIA_PLAY_PAUSE, KC_MEDIA_PLAY_PAUSE, KC_MEDIA_NEXT_TRACK},
		.right_encoder_map = {KC_APP_CHROME_FORWARD, KC_APP_CHROME_BACKWARD, KC_APP_CHROME_NEW_TAB, KC_APP_CHROME_NEW_WINDOW, KC_APP_CHROME_CLOSE_TAB},
		// APDS9960 -  {UP, DOWN, LEFT, RIGHT, NEAR, FAR}
		.gesture_map = {KC_APP_CHROME_CLOSE_TAB, 0, KC_APP_CHROME_NTAB, KC_APP_CHROME_PTAB, 0, 0},
		.active = true,
};


dd_layer *default_layouts[LAYERS] = {&layer1, &layer2, &layer3};
uint8_t current_layout = 0;

dd_macros default_macros[MACROS_NUM]={0};
dd_macros *ptr_default_macros [MACROS_NUM];

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
