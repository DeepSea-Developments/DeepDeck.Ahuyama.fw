/**
 * @file deepdeck_tasks.c
 * @author ElectroNick (nick@dsd.dev)
 * @brief source file of the deepdeck tasks
 * @version 0.1
 * @date 2022-12-08
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "deepdeck_tasks.h"
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_sleep.h"

// MK32 functions
#include "keypress_handles.c"
#include "battery_monitor.h"
#include "nvs_funcs.h"

#include "esp_err.h"

#include "rotary_encoder.h"
#include "rgb_led.h"
#include "menu.h"

// HID Ble functions
#include "hal_ble.h"

#include "keycode_conv.h"

#include "gesture_handles.h"

static const char *TAG = "KeyReport";

#define KEY_REPORT_TAG "KEY_REPORT"
#define SYSTEM_REPORT_TAG "KEY_REPORT"
#define SCREENSAVER_TAG "SCREENSAVER"
// #define TRUNC_SIZE 20
#define USEC_TO_SEC 1000000
#define SEC_TO_MIN 60

#ifdef OLED_ENABLE
TaskHandle_t xOledTask;
#endif

TaskHandle_t xKeyreportTask;
deepdeck_status_t deepdeck_status = S_NORMAL; // Set the status of the screen.

// extern SemaphoreHandle_t xSemaphore;

/**
 * @todo look a better way to handle the deepsleep flag.
 *
 */
bool DEEP_SLEEP = true; // flag to check if we need to go to deep sleep

/**
 * @brief Idle seconds before the OLED is blanked. 0 disables the screensaver.
 *
 * Seeded from SCREENSAVER_SECS, overwritten at boot with whatever is in NVS,
 * and changed at runtime from the OLED menu.
 */
#ifdef SCREENSAVER_SECS
uint16_t screensaver_timeout_sec = SCREENSAVER_SECS;
#else
uint16_t screensaver_timeout_sec = 0;
#endif

/** @brief Set by every input handler to tell the screensaver task "not idle". */
static volatile bool screensaver_activity = true;

void screensaver_notify_activity(void)
{
	screensaver_activity = true;
}

bool screensaver_is_blanked(void)
{
	return deepdeck_status == S_SCREENSAVER;
}

bool screensaver_wake(void)
{
	screensaver_notify_activity();

	if (deepdeck_status == S_SCREENSAVER)
	{
		deepdeck_status = S_NORMAL;
		return true;
	}

	return false;
}

void screensaver_set_timeout_sec(uint16_t seconds)
{
	screensaver_timeout_sec = seconds;
	screensaver_notify_activity();

	// Turning the screensaver off while the screen is already blanked has to
	// bring it back, otherwise nothing would ever un-blank it again.
	if (seconds == 0 && deepdeck_status == S_SCREENSAVER)
	{
		deepdeck_status = S_NORMAL;
	}
}

uint16_t screensaver_get_timeout_sec(void)
{
	return screensaver_timeout_sec;
}

void oled_task(void *pvParameters)
{
	deepdeck_status = S_NORMAL; // sSet the status of the screen.
	ble_connected_oled();
	bool CON_LOG_FLAG = false; // Just because I don't want it to keep logging the same thing a billion times
	bool oled_powered_down = false;

	while (1)
	{
		// The panel has to be awake before anything is drawn, whichever state
		// we are moving into - the menu and the "waiting for connection" screen
		// are just as invisible on a powered down OLED as the normal view.
		// Only the S_SCREENSAVER case below powers it back down.
		if (oled_powered_down && deepdeck_status != S_SCREENSAVER)
		{
			u8g2_SetPowerSave(&u8g2, 0);
			oled_powered_down = false;
		}

		switch (deepdeck_status)
		{
		case S_NORMAL: // Normal mode, showing the keys
			if (halBLEIsConnected() == 0)
			{
				if (CON_LOG_FLAG == false)
				{
					ESP_LOGI(TAG, "Not connected, waiting for connection ");
				}
				waiting_oled();
				DEEP_SLEEP = false;
				CON_LOG_FLAG = true;
			}
			else
			{
				/* The wifi address usually arrives seconds after the BLE
				 * connection, long after the banner was last drawn, so a
				 * redraw has to be triggered when it changes or the screen
				 * keeps showing the placeholder. */
				if (CON_LOG_FLAG == true || oled_wifi_status_changed())
				{
					ble_connected_oled();
				}
				update_oled();
				CON_LOG_FLAG = false;
			}
			break;
		case S_SETTINGS: // Settings mode, showing the internal menu
			disable_interrup_pin();
			menu_init();

			vTaskDelay(pdMS_TO_TICKS(200));

			menu_screen();
			ble_connected_oled();

			deepdeck_status = S_NORMAL;
			// release gesture sensor again
			// apds9960_free();
			config_interrup_pin();
			break;
		case S_SCREENSAVER:
			// Power the panel down once, not on every pass of the loop - each
			// call is an I2C transaction on the bus shared with the gesture sensor.
			if (oled_powered_down == false)
			{
				u8g2_SetPowerSave(&u8g2, 1);
				oled_powered_down = true;
			}
			break;
		}
		vTaskDelay(pdMS_TO_TICKS(100));
	}
}

void gesture_task(void *pvParameters)
{

	while (true)
	{

		if (xSemaphoreTake(xSemaphore, 10) == pdTRUE)
		{

			// Do not send anything if queues are uninitialized
			if (keyboard_q == NULL || joystick_q == NULL)
			{
				ESP_LOGE(TAG, "queues not initialized");
				continue;
			}

			// ESP_LOGW("Gesture", "xSemaphore Take");
			do
			{
				vTaskDelay(pdMS_TO_TICKS(10));
			} while (deepdeck_status == S_SETTINGS);
			read_gesture();
		}
		else
		{
#ifdef PROXIMITY_WAKE
			/* Roughly every 70ms. Each pass of this loop is about 35ms, and
			 * the check itself does nothing unless the panel is blanked - at
			 * which point the OLED is powered down and not using the I2C bus,
			 * so this contends with nothing. Polling faster costs nothing real
			 * and shortens the delay before the screen comes back. */
			static uint8_t proximity_tick = 0;

			if (++proximity_tick >= 2)
			{
				proximity_tick = 0;
				gesture_proximity_wake_check();
			}
#endif
			vTaskDelay(pdMS_TO_TICKS(10));
		}

		vTaskDelay(pdMS_TO_TICKS(10));
	}
}

void battery_reports(void *pvParameters)
{
	// uint8_t past_battery_report[1] = { 0 };

	while (1)
	{
		uint32_t bat_level = get_battery_level();
		// if battery level is above 100, we're charging
		if (bat_level > 100)
		{
			bat_level = 100;
			// if charging, do not enter deepsleep
			DEEP_SLEEP = false;
		}
		void *pReport = (void *)&bat_level;

		ESP_LOGI("Battery Monitor", "battery level %" PRIu32, bat_level);
		if (BLE_EN == 1)
		{
			xQueueSend(battery_q, pReport, (TickType_t)0);
		}
		if (input_str_q != NULL)
		{
			xQueueSend(input_str_q, pReport, (TickType_t)0);
		}
		vTaskDelay(60 * 1000 / portTICK_PERIOD_MS);
	}
}

void key_reports(void *pvParameters)
{
	// Arrays for holding the report at various stages
	uint8_t past_report[REPORT_LEN] = {0};
	uint8_t report_state[REPORT_LEN];

	while (1)
	{
		memcpy(report_state, check_key_state(&key_layouts[current_layout]),
			   sizeof report_state);

		// Do not send anything if queues are uninitialized
		if (mouse_q == NULL || keyboard_q == NULL || joystick_q == NULL)
		{
			ESP_LOGE(TAG, "queues not initialized");
			continue;
		}

		// Check if the report was modified, if so send it
		if (memcmp(past_report, report_state, sizeof past_report) != 0)
		{
			// wake up
			DEEP_SLEEP = false;
			screensaver_wake();

			void *pReport;
			memcpy(past_report, report_state, sizeof past_report);

#ifndef NKRO
			uint8_t trunc_report[REPORT_LEN] = {0};
			trunc_report[0] = report_state[0];
			trunc_report[1] = report_state[1];

			uint16_t cur_index = 2;
			// Phone's mtu size is usuaully limited to 20 bytes
			for (uint16_t i = 2; i < REPORT_LEN && cur_index < TRUNC_SIZE;
				 ++i)
			{
				if (report_state[i] != 0)
				{
					trunc_report[cur_index] = report_state[i];
					++cur_index;
				}
			}

			pReport = (void *)&trunc_report;
#endif
#ifdef NKRO
			pReport = (void *)&report_state;
#endif

			if (BLE_EN == 1)
			{
				xQueueSend(keyboard_q, pReport, (TickType_t)0);
			}
			if (input_str_q != NULL)
			{
				xQueueSend(input_str_q, pReport, (TickType_t)0);
			}
		}
		vTaskDelay(pdMS_TO_TICKS(10));
	}
}

void rgb_leds_task(void *pvParameters)
{

	rgb_key_led_init();
	rgb_notification_led_init();
	while (1)
	{
		key_led_modes();
	}
}
extern uint8_t gesture_disable_g;
void encoder_report(void *pvParameters)
{
	uint8_t encoder1_status = 0;
	uint8_t encoder2_status = 0;
	uint8_t past_encoder1_state = 0;
	uint8_t past_encoder2_state = 0;

	rotary_encoder_t *encoder_a = NULL;
	rotary_encoder_t *encoder_b = NULL;

	//--------------------Start encoder 1---------------
	// Rotary encoder underlying device is represented by a PCNT unit in this example
	uint32_t pcnt_unit_a = 0;

	// Create rotary encoder instance
	rotary_encoder_config_t config_a =
		ROTARY_ENCODER_DEFAULT_CONFIG((rotary_encoder_dev_t)pcnt_unit_a, ENCODER1_A_PIN, ENCODER1_B_PIN, ENCODER1_S_PIN, ENCODER1_S_ACTIVE_LOW);
	ESP_ERROR_CHECK(rotary_encoder_new_ec11(&config_a, &encoder_a));

	// Filter out glitch (1us)
	ESP_ERROR_CHECK(encoder_a->set_glitch_filter(encoder_a, 1));

	// Start encoder
	ESP_ERROR_CHECK(encoder_a->start(encoder_a));

	//--------------------Start encoder 2---------------
	uint32_t pcnt_unit_b = 1;

	// Create rotary encoder instance
	rotary_encoder_config_t config_b =
		ROTARY_ENCODER_DEFAULT_CONFIG((rotary_encoder_dev_t)pcnt_unit_b, ENCODER2_A_PIN, ENCODER2_B_PIN, ENCODER2_S_PIN, ENCODER2_S_ACTIVE_LOW);
	ESP_ERROR_CHECK(rotary_encoder_new_ec11(&config_b, &encoder_b));

	// Filter out glitch (1us)
	ESP_ERROR_CHECK(encoder_b->set_glitch_filter(encoder_b, 1));

	// Start encoder
	ESP_ERROR_CHECK(encoder_b->start(encoder_b));

	while (1)
	{
		encoder1_status = encoder_state(encoder_a);

		if (encoder1_status != past_encoder1_state)
		{
			DEEP_SLEEP = false;
			screensaver_wake();

			//  Check if both encoder are pushed, to enter settings mode.

			if (deepdeck_status == S_SETTINGS)
			{

				menu_command((encoder_state_t)encoder1_status);
			}
			else if (encoder1_status == ENC_BUT_LONG_PRESS && encoder_push_state(encoder_b))
			{
				// Enter Setting mode.
				deepdeck_status = S_SETTINGS;
				ESP_LOGI("Encoder 1", "setting mode");
			}
			else
			{
				encoder_command(encoder1_status,
								key_layouts[current_layout].left_encoder_map);
			}
			past_encoder1_state = encoder1_status;
		}

		encoder2_status = encoder_state(encoder_b);

		if (encoder2_status != past_encoder2_state)
		{

			if ((encoder2_status != ENC_IDLE) && (!gesture_disable_g))
			{
				ESP_LOGI("Encoder 2", "USED");
				gesture_disable_g = 1;
			}

			DEEP_SLEEP = false;
			screensaver_wake();

			// Check if both encoder are pushed, to enter settings mode.
			if (encoder2_status == ENC_BUT_LONG_PRESS && encoder_push_state(encoder_a))
			{
				// Enter Setting mode.
				deepdeck_status = S_SETTINGS;
				ESP_LOGI("Encoder 2", "setting mode");
			}
			else
			{
				encoder_command(encoder2_status,
								key_layouts[current_layout].right_encoder_map);
			}

			past_encoder2_state = encoder2_status;
		}
		taskYIELD();
	}
}

/*If no key press has been recieved in SLEEP_MINS amount of minutes, put device into deep sleep
 *  wake up on touch on GPIO pin 2
 *  */
#ifdef SLEEP_MINS
void deep_sleep(void *pvParameters)
{

	uint64_t initial_time = esp_timer_get_time(); // notice that timer returns time passed in microseconds!
	uint64_t current_time_passed = 0;
	uint8_t force_sleep = false;
	while (1)
	{
		current_time_passed = (esp_timer_get_time() - initial_time);

		if (DEEP_SLEEP == false)
		{
			current_time_passed = 0;
			initial_time = esp_timer_get_time();
			DEEP_SLEEP = true;
		}
		if (menu_get_goto_sleep())
		{
			force_sleep = true;
			DEEP_SLEEP = true;
		}

		if ((((double)current_time_passed / USEC_TO_SEC) >= (double)(SEC_TO_MIN * SLEEP_MINS)) || force_sleep)
		{
			if (DEEP_SLEEP == true)
			{
				force_sleep = false;
				ESP_LOGE(SYSTEM_REPORT_TAG, "going to sleep!");
#ifdef OLED_ENABLE
				vTaskDelay(20 / portTICK_PERIOD_MS);
				vTaskSuspend(xOledTask);
				deinit_oled();
#endif
				// wake up esp32 using rtc gpio
				rtc_matrix_setup();
				esp_sleep_enable_touchpad_wakeup();
				esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
				esp_deep_sleep_start();
			}
			if (DEEP_SLEEP == false)
			{
				current_time_passed = 0;
				initial_time = esp_timer_get_time();
				DEEP_SLEEP = true;
			}
		}

		vTaskDelay(pdMS_TO_TICKS(10));
	}
}
#endif

/* Blank the OLED after screensaver_timeout_sec seconds without input.
 * Any key press or knob movement wakes it back up, via screensaver_wake().
 *  */
#ifdef SCREENSAVER_SECS
#ifdef OLED_ENABLE
void screensaver(void *pvParameters)
{
	uint64_t last_activity = esp_timer_get_time(); // the timer returns microseconds
	uint64_t idle_time = 0;

	// A saved value overrides the SCREENSAVER_SECS default; if nothing has ever
	// been saved, nvs_load_screensaver_secs leaves the variable alone.
	nvs_load_screensaver_secs(&screensaver_timeout_sec);
	ESP_LOGI(SCREENSAVER_TAG, "timeout is %d sec", screensaver_timeout_sec);

	while (1)
	{
		if (screensaver_activity == true)
		{
			screensaver_activity = false;
			last_activity = esp_timer_get_time();
		}

		// Only count idle time in the normal view: 0 means the user turned the
		// screensaver off, and blanking the screen out from under the settings
		// menu would leave them navigating blind.
		if (screensaver_timeout_sec > 0 && deepdeck_status == S_NORMAL)
		{
			idle_time = esp_timer_get_time() - last_activity;

			if (idle_time >= (uint64_t)screensaver_timeout_sec * USEC_TO_SEC)
			{
				ESP_LOGI(SCREENSAVER_TAG, "enabling screensaver");
				deepdeck_status = S_SCREENSAVER;
			}
		}

		vTaskDelay(pdMS_TO_TICKS(100));
	}
}
#endif
#endif
