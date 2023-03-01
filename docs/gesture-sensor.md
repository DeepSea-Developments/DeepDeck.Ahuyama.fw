# Gesture Sensor 

DeepDeck has a apds9960 Digital Proximity, Ambient Light, RGB and Gesture Sensor. Gesture detection utilizes four directional photodiodes to
sense reflected IR energy (sourced by the integrated LED) to convert physical motion information (i.e. velocity, direction and distance) to a digital information. This allows having the following interactions:

* UP and DOWN Gesture
* LEFT and RIGHT Gesture
* NEAR and FAR Gesture

To perform a up, down, left and right gestures, swipe your hand over the sensor.

To perform a NEAR gesture, hold your hand far above the sensor and move it close to the sensor (within 2 inches). Hold your hand there for at least 1 second and move it away.

To perform a FAR gesture, hold your hand within 2 inches of the sensor for at least 1 second and then move it above (out of
range) of the sensor.

# Gesture Configuration 
Similar to the layers, the gesture configurations can be changed with the currently used layer.
You can change  and `.gesture_map` on each layer. They are in the file `keymap.c`.

``` c
		// APDS9960 -  {UP, DOWN, LEFT, RIGHT, NEAR, FAR}
		.gesture_map = {KC_AUDIO_VOL_DOWN, KC_AUDIO_VOL_UP, KC_MEDIA_PLAY_PAUSE, KC_AUDIO_MUTE, KC_MEDIA_NEXT_TRACK, KC_MEDIA_NEXT_TRACK},

```

# Known Issues
While usign the right enfoder the sensor detect false getures. to avoid this a one-shot software-timer was implemented in  `gesture_handles.c`.
edit `pdMS_TO_TICKS(750)),` to adjust the reload time.

``` c
esp_err_t set_timer(void) {
	xTimer = xTimerCreate("Timer", // Just a text name, not used by the kernel.
			(pdMS_TO_TICKS(750)),   // The timer period in ticks.
			pdFALSE, //pdTRUE The timers will auto-reload themselves when they expire.
			(void*) timerID, // Assign each timer a unique id equal to its array index.
			vTimerCallback // Each timer calls the same callback when it expires.
			);

	if (xTimer == NULL) {
		ESP_LOGI(".", "The timer was not created");
		// The timer was not created.
	} else {
		// Start the timer.  No block time is specified, and even if one was
		// it would be ignored because the scheduler has not yet been
		// started.
		if ( xTimerStart( xTimer, 0 ) != pdPASS) {
			ESP_LOGI(".", "The timer could not be set into the Active state");
			// The timer could not be set into the Active state.
		}
	}
	return ESP_OK;
}
``` 
# TO DO:
* Improve FAR and NEAR detection
* Improve false gesture detection while using right encoder
