# Macro configuration

DeepDeck can map several keys to one. Commands like ctrl+c or ctrl+alt+del can be mapped into one single action, a macro. 

The configuration of the macros is in `keymap.c`.

## Adding a Macro
Defining a macro consists of 3 steps:

1. Name the macro
1. Define the keys that contain
1. Use the macro.

### Name the macro

In the array `custom_macros` you can add the new macro. The order of the macro should be the same as the next step. Just to be sure, add your new macro at the end of the list.
``` c
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
```

### Define the macro

Add a new item to the `macros` list. This list is defined by the number of available macros `MACROS_NUM` and the length of a macro `MACRO_LEN`. You can change these definitions in `keyboard_config.h`.

If the macro length is less than `MACRO_LEN`, you have to use the definition `KC_NO` to let the program know the macro is over.

``` c
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
```

### Use the macro

Now you can integrate the macro (with the name defined in the `custom_macro` list) on the layers or the knobs.