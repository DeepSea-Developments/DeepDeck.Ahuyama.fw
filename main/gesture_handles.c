/*
 * gesture_test.c
 *
 *  Created on: Feb 4, 2023
 *      Author: mauro
 */

// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include "gesture_handles.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "freertos/timers.h"
// HID Ble functions
#include "hal_ble.h"
#include "keypress_handles.h"
#include "keyboard_config.h"

TaskHandle_t xGesture;

i2c_bus_handle_t i2c_bus = NULL;
apds9960_handle_t apds9960 = NULL;
TimerHandle_t xTimer;
int timerID = 1;
bool flag = false;

///////////////////////////////////
void vTimerCallback(TimerHandle_t pxTimer)
{
	//	ESP_LOGI(".", "vTimerCallback");
	flag = true;
}

//////////////////////////////////
esp_err_t set_timer(void)
{
	xTimer = xTimerCreate("Timer",				// Just a text name, not used by the kernel.
						  (pdMS_TO_TICKS(750)), // The timer period in ticks.
						  pdFALSE,				// pdTRUE The timers will auto-reload themselves when they expire.
						  (void *)timerID,		// Assign each timer a unique id equal to its array index.
						  vTimerCallback		// Each timer calls the same callback when it expires.
	);

	if (xTimer == NULL)
	{
		ESP_LOGI(".", "The timer was not created");
		// The timer was not created.
	}
	else
	{
		// Start the timer.  No block time is specified, and even if one was
		// it would be ignored because the scheduler has not yet been
		// started.
		if (xTimerStart(xTimer, 0) != pdPASS)
		{
			ESP_LOGI(".", "The timer could not be set into the Active state");
			// The timer could not be set into the Active state.
		}
	}
	return ESP_OK;
}
///////////////////////////////////
void IRAM_ATTR gesture_isr_handler(void *arg)
{
	BaseType_t task_woken = pdFALSE;
	//	uint32_t gpio_num = (uint32_t) arg;
	//	gpio_intr_disable(gpio_num);

	xSemaphoreGiveFromISR(xSemaphore, &task_woken);

	if (task_woken)
		portYIELD_FROM_ISR();
}
///////////////////////////////////

/**
 * @brief i2c master initialization
 */
void apds9960_init(i2c_bus_handle_t *i2cbus)
{

	set_timer();
	config_interrup_pin();
	i2c_bus = *i2cbus;
	apds9960 = apds9960_create(i2c_bus, APDS9960_I2C_ADDRESS);

	apds9960_gesture_init(apds9960);
}

void apds9960_deinit()
{
	apds9960_delete(&apds9960);
	i2c_bus_delete(&i2c_bus);
}

void apds9960_free()
{
	apds9960_gesture_init(apds9960);
}

void read_gesture()
{
	uint8_t gesture = 0;

		//			ESP_LOGI("Gesture", "Read Gesture start .......");
		gesture = apds9960_read_gesture(apds9960);
		if (gesture != APDS9960_NONE)
		{
			if (gesture == APDS9960_DOWN)
			{
				ESP_LOGE("Gesture", "_DOWN");
			}
			else if (gesture == APDS9960_UP)
			{
				ESP_LOGE("Gesture", "_UP");
			}
			else if (gesture == APDS9960_LEFT)
			{
				ESP_LOGE("Gesture", "_LEFT");
			}
			else if (gesture == APDS9960_RIGHT)
			{
				ESP_LOGE("Gesture", "_RIGHT");
			}
			else if (gesture == APDS9960_FAR)
			{
				ESP_LOGE("Gesture", "_FAR");
			}
			else if (gesture == APDS9960_NEAR)
			{
				ESP_LOGE("Gesture", "_NEAR");
			}
			gesture_command(gesture,
							key_layouts[current_layout].gesture_map);
		}
		else
		{
			// ESP_LOGE("Gesture", "_NONE");
		}

}

// How to process gesture activity
void gesture_command(uint8_t command, uint16_t gesture_commands[GESTURE_SIZE])
{
	uint8_t modifier = 0;
	uint16_t action;
	uint8_t key_state[REPORT_LEN] = {0};

	if (command != 0)
	{
		action = gesture_commands[command - 1]; // Have to substract 1 as IDLE is the state 0.

		// checking for layer adjust keycodes
		if ((action > LAYER_ADJUST_MIN) && (action < LAYER_ADJUST_MAX))
		{
			ESP_LOGI("gesture", "Layer Adjust: %d", action);
			layer_adjust(action);
			// continue;
		}

		// checking for macros
		if ((action >= MACRO_BASE_VAL) && (action <= MACRO_HOLD_MAX_VAL))
		{
			ESP_LOGI("gesture", "MACRO: %d", action);
			for (uint8_t i = 0; i < MACRO_LEN; i++)
			{
				uint16_t key = user_macros[action - MACRO_BASE_VAL].key[i];

				if (key == KC_NO)
				{
					break;
				}
				// ESP_LOGI("PressMacro", "keycode: %d", keycode - MACRO_BASE_VAL);
				key_state[i + 2] = key; // 2 is an offset, as 0 and 1 are used for other reasons
				modifier |= check_modifier(key);
			}
			key_state[0] = modifier;
			xQueueSend(keyboard_q, (void *)&key_state, (TickType_t)0);

			// release macro
			for (uint8_t i = 0; i < MACRO_LEN; i++)
			{
				// uint16_t key = macros[keycode - MACRO_BASE_VAL][i];
				uint16_t key = user_macros[action - MACRO_BASE_VAL].key[i];

				// ESP_LOGI("releaseMacro", "keycode: %d", keycode - MACRO_BASE_VAL);
				key_state[i + 2] = 0; // 2 is an offset, as 0 and 1 are used for other reasons
				// ESP_LOGI("releaseMacro", "report_id: %d", i + 2);
				modifier &= ~check_modifier(key);
				// ESP_LOGI("releaseMacro", "Key: %d", key);
			}
			key_state[0] = modifier;
			xQueueSend(keyboard_q, (void *)&key_state, (TickType_t)0);
		}

		// checking media actions
		if (action >= KC_MEDIA_NEXT_TRACK && action <= KC_MEDIA_REWIND)
		{
			ESP_LOGI("gesture", "Media Control: %d", action);
			media_control_send(action);
			media_control_release(action);
		}

		// Otherwise Regular keys
		else
		{
			ESP_LOGI("Gesture", "Regular key detected: %d", action);
			key_state[2] = action;
			xQueueSend(keyboard_q, (void *)&key_state, (TickType_t)0);
			key_state[2] = 0;
			xQueueSend(keyboard_q, (void *)&key_state, (TickType_t)0);
		}
	}

	vTaskDelay(5 / portTICK_PERIOD_MS);
}


////Config Interrup PIN
void config_interrup_pin(void)
{

	gpio_num_t interrupt_pin = (gpio_num_t)INTERRUPT_GPIO;
	gpio_config_t io_conf;
	io_conf.intr_type = GPIO_INTR_NEGEDGE; // GPIO_INTR_ANYEDGE;//
	io_conf.pin_bit_mask = (1ULL << INTERRUPT_GPIO);
	io_conf.mode = GPIO_MODE_INPUT;
	io_conf.pull_up_en = GPIO_PULLUP_ENABLE; // (gpio_pullup_t) 0;
	io_conf.pull_down_en = (gpio_pulldown_t)0;
	gpio_config(&io_conf);

	// install gpio isr service
	gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
	// hook isr handler for specific gpio pin
	gpio_isr_handler_add(interrupt_pin, gesture_isr_handler,
						 (void *)interrupt_pin);
}

void disable_interrup_pin(void)
{
	gpio_num_t interrupt_pin = (gpio_num_t)INTERRUPT_GPIO;
	// zero-initialize the config structure.
	gpio_config_t io_conf = {};
	// disable interrupt
	io_conf.intr_type = GPIO_INTR_DISABLE;
	// set as output mode
	io_conf.mode = GPIO_MODE_DEF_DISABLE;
	// bit mask of the pins that you want to set,e.g.GPIO18/19
	io_conf.pin_bit_mask = (1ULL << INTERRUPT_GPIO);
	// disable pull-down mode
	io_conf.pull_down_en = 0;
	// disable pull-up mode
	io_conf.pull_up_en = 0;
	// configure GPIO with the given settings
	gpio_config(&io_conf);
	gpio_isr_handler_remove(interrupt_pin);
	gpio_uninstall_isr_service();
	
}
