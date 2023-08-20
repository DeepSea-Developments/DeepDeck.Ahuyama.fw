# Layer configurations

DeepDeck consists of 4x4 mechanical keys. A specific configuration of those 16 keys is called a layer. By default, there are 3 layers, but you can have more.

The configuration of the layers is in `keymap.c`

Here is an example of the layer configuration

``` c
// Each keymap is represented by an array, with an array that points to all the keymaps  in order
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
```

The array consists of either macros or key bindings. The complete list of keys is in the file `key_definitions.h`. This also includes special keys, or actions like mouse right/left clicks.

## Change the name of the layers

To change the name, go to the array `default_layout_names`. You can also add more layers by changing the definition `LAYERS` that is in `keyboard_config.h`
``` c
// array to hold names of layouts for oled
char default_layout_names[LAYERS][MAX_LAYOUT_NAME_LENGTH] = { "MEDIA", "NUM",
		  "Plugins",
		};
```

## Add new layers

If you want to add more than 3 layers, or have different layers defined, but just use 3 at the time, add/change the names on the `default_layouts` array.
``` c
 //Create an array that points to the various keymaps
uint16_t (*default_layouts[])[MATRIX_ROWS][KEYMAP_COLS] = { &_QWERTY, &_NUM,
			&_PLUGINS
		};
```

## Change the layer

Now you can change the layer. The top chart is just a guide to remember layer, the thing that matters is the list below it (KC_1, etc).

``` c title="bubble_sort.py" linenums="1"
uint16_t _NUM[MATRIX_ROWS][KEYMAP_COLS]={

	/* NUM
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
S
};
```

## Test your layers

After this, you can run the fire :fire: icon on the bar below to build, flash, and monitor. Make sure everything is working correctly :smile: 
