# Knob configuration

DeepDeck has 2 rotary knobs with integrated buttons. This allows having the following interactions:

* ClockWise rotation
* CounterClockWise rotation
* Single button press
* Double-press button
* Long-press button

Similar to the layers, the knob configurations can be changed with the currently used layer.
You can change `default_encoder_map` and `default_slave_encoder_map` to modify left and right encoders. They are in the file `keymap.c`.

``` c
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
};
```