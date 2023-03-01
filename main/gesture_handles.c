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
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "freertos/timers.h"
//HID Ble functions
#include "hal_ble.h"

TaskHandle_t xGesture;

i2c_bus_handle_t i2c_bus = NULL;
apds9960_handle_t apds9960 = NULL;
xTimerHandle xTimer;
int timerID = 1;
bool flag = false;


///////////////////////////////////
void vTimerCallback(TimerHandle_t pxTimer) {
//	ESP_LOGI(".", "vTimerCallback");
	flag = true;
}
//////////////////////////////////
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
///////////////////////////////////
void IRAM_ATTR gesture_isr_handler(void *arg) {
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
void apds9960_init() {
	set_timer();
	config_interrup_pin();
	//TO DO -improve this.
	// this is also for oled  initialization. 
	int i2c_master_port = APDS9960_I2C_MASTER_NUM;
	i2c_config_t conf = { .mode = I2C_MODE_MASTER, .sda_io_num =
	APDS9960_I2C_MASTER_SDA_IO, .sda_pullup_en = GPIO_PULLUP_ENABLE,
			.scl_io_num = APDS9960_I2C_MASTER_SCL_IO, .scl_pullup_en =
					GPIO_PULLUP_ENABLE, .master.clk_speed =
			APDS9960_I2C_MASTER_FREQ_HZ, };
	i2c_bus = i2c_bus_create(i2c_master_port, &conf);
	apds9960 = apds9960_create(i2c_bus, APDS9960_I2C_ADDRESS);

	apds9960_gesture_init(apds9960);
}

void apds9960_deinit() {
	apds9960_delete(&apds9960);
	i2c_bus_delete(&i2c_bus);
}


void apds9960_free(){
	apds9960_gesture_init(apds9960);

}

void read_gesture() {
	uint8_t gesture = 0;
//flag to prevent false gestures while using the left encoder
	if (flag) {

			flag = false;
//			ESP_LOGI("Gesture", "Suspend xOledTask");
			vTaskSuspend(xOledTask);//suspend oled task to prevent i2c bus crashed

//			ESP_LOGI("Gesture", "Read Gesture start .......");
			gesture = apds9960_read_gesture(apds9960);
			if (gesture != APDS9960_NONE) {
				if (gesture == APDS9960_DOWN) {
					ESP_LOGI("Gesture", "_DOWN");

				} else if (gesture == APDS9960_UP) {
					ESP_LOGI("Gesture", "_UP");

				} else if (gesture == APDS9960_LEFT) {
					ESP_LOGI("Gesture", "_LEFT");

				} else if (gesture == APDS9960_RIGHT) {
					ESP_LOGI("Gesture", "_RIGHT");

				} else if (gesture == APDS9960_FAR) {
					ESP_LOGI("Gesture", "_FAR");

				} else if (gesture == APDS9960_NEAR) {
					ESP_LOGI("Gesture", "_NEAR");

				}
				gesture_command(gesture,
						key_layouts[current_layout].gesture_map);

			}

			xTimerStart(xTimer, 0);
			vTaskResume(xOledTask);


	} else { //reload the timer
		xTimerStop(xTimer, 0);///stop the timer
		vTaskSuspend(xOledTask);// suspend oled to be able to send i2c cmd to the gesture sensor
//		ESP_LOGI(".", "--");
		apds9960_gesture_init(apds9960);
		xTimerStart(xTimer, 0);///start the timer
		vTaskResume(xOledTask);


	}

}

//How to process gesture activity
void gesture_command(uint8_t command, uint16_t gesture_commands[GESTURE_SIZE]) {

	uint16_t action;
	uint8_t media_state[2] = { 0, 0 };
//    uint8_t mouse_state[5] = {0,0,0,0,0};
	uint8_t key_state[REPORT_LEN] = { 0 };
//    uint8_t modifier = 0;

	if (command != 0) {
		action = gesture_commands[command - 1]; //Have to substract 1 as IDLE is the state 0.

		//Review media actions
		if (action >= KC_MEDIA_NEXT_TRACK && action <= KC_MEDIA_REWIND) {
			switch (action) {
			case KC_MEDIA_NEXT_TRACK:
				media_state[1] = 10;
				break;

			case KC_MEDIA_PREV_TRACK:
				media_state[1] = 111;
				break;

			case KC_MEDIA_STOP:
				media_state[1] = 12;
				break;

			case KC_MEDIA_PLAY_PAUSE:
				media_state[1] = 5;
				break;

			case KC_AUDIO_MUTE:
				media_state[1] = 1;
				break;

			case KC_AUDIO_VOL_UP:
				SET_BIT(media_state[0], 6)
				;
				break;

			case KC_AUDIO_VOL_DOWN:
				SET_BIT(media_state[0], 7)
				;
				break;
			}

			xQueueSend(media_q, (void* )&media_state, (TickType_t ) 0); // Send Command
			media_state[0] = 0;
			media_state[1] = 0;
			xQueueSend(media_q, (void* )&media_state, (TickType_t ) 0); // Reset command
		}
		//Review mouse actions
//        else if(action >= KC_MS_UP && action <= KC_MS_ACCEL2)
//        {
//            switch(action)
//            {
//                case KC_MS_UP:
//                    mouse_state[2] = 15;
//                    break;
//
//                case KC_MS_DOWN:
//                    mouse_state[2] = -15;
//                    break;
//
//                case KC_MS_LEFT:
//                    mouse_state[1] = -15;
//                    break;
//
//                case KC_MS_RIGHT:
//                    mouse_state[1] = 15;
//                    break;
//
//                case KC_MS_BTN1:
//                    mouse_state[0] = 1;
//                    break;
//
//                case KC_MS_BTN2:
//                    mouse_state[0] = 2;
//                    break;
//
//                case KC_MS_WH_UP:
//                    mouse_state[3] = 1;
//                    break;
//                case KC_MS_WH_DOWN:
//                    mouse_state[3] = -1;
//                    break;
//            }
//            xQueueSend(mouse_q,(void*)&mouse_state, (TickType_t) 0);
//            mouse_state[0] = 0;
//            mouse_state[1] = 0;
//            mouse_state[2] = 0;
//            mouse_state[3] = 0;
//            xQueueSend(mouse_q,(void*)&mouse_state, (TickType_t) 0);
//        }
		//Review Macro actions
//        else if(action >= MACRO_BASE_VAL && action < MACRO_BASE_VAL + MACROS_NUM)
//        {
//            //ESP_LOGI("Encoder","Macro detected %d", action);
//            uint8_t i;
//            for (i = 0; i < MACRO_LEN; i++) {
//                uint16_t key = macros[action - MACRO_BASE_VAL][i];
//                if (key == KC_NO)
//                {
//                    break;
//                }
//                key_state[2+i] = key;
//                modifier |= check_key_modifier(key);
//
//                //ESP_LOGI("KEY sent", "macroid: %d", key);
//            }
//            key_state[0] = modifier;
//            xQueueSend(keyboard_q,(void*)&key_state, (TickType_t) 0);
//            for (i = 0; i < MACRO_LEN; i++) {
//                uint16_t key = macros[action - MACRO_BASE_VAL][i];
//                if (key == KC_NO)
//                {
//                    break;
//                }
//                key_state[2+i] = 0;
//                modifier &= ~check_key_modifier(key);
//            }
//            key_state[0] = modifier;
//            xQueueSend(keyboard_q,(void*)&key_state, (TickType_t) 0);
//        }
		//Review Key actions
		else {
			ESP_LOGI("Gesture", "Regular key detected: %d", action);
			key_state[2] = action;
			xQueueSend(keyboard_q, (void* )&key_state, (TickType_t ) 0);
			key_state[2] = 0;
			xQueueSend(keyboard_q, (void* )&key_state, (TickType_t ) 0);
		}
	}

	vTaskDelay(5 / portTICK_PERIOD_MS);
}

void apds9960_test_gesture() {
	int cnt = 0;
	while (cnt < 10) {
		uint8_t gesture = apds9960_read_gesture(apds9960);
		if (gesture == APDS9960_DOWN) {
			printf("gesture APDS9960_DOWN*********************!\n");
		} else if (gesture == APDS9960_UP) {
			printf("gesture APDS9960_UP*********************!\n");
		} else if (gesture == APDS9960_LEFT) {
			printf("gesture APDS9960_LEFT*********************!\n");
			cnt++;
		} else if (gesture == APDS9960_RIGHT) {
			printf("gesture APDS9960_RIGHT*********************!\n");
			cnt++;
		}
		//vTaskDelay(100 / portTICK_RATE_MS);
	}
}

////Config Interrup PIN
void config_interrup_pin(void) {

	gpio_num_t interrupt_pin = (gpio_num_t) INTERRUPT_GPIO;
	gpio_config_t io_conf;
	io_conf.intr_type = GPIO_INTR_NEGEDGE; //GPIO_INTR_ANYEDGE;//
	io_conf.pin_bit_mask = (1ULL << INTERRUPT_GPIO);
	io_conf.mode = GPIO_MODE_INPUT;
	io_conf.pull_up_en = GPIO_PULLUP_ENABLE; // (gpio_pullup_t) 0;
	io_conf.pull_down_en = (gpio_pulldown_t) 0;
	gpio_config(&io_conf);

//install gpio isr service
	gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
//hook isr handler for specific gpio pin
	gpio_isr_handler_add(interrupt_pin, gesture_isr_handler,
			(void*) interrupt_pin);

}

///////////////

void test() {

	apds9960_init();
//	apds9960_gesture_init(apds9960);
	vTaskDelay(pdMS_TO_TICKS(1000));
	apds9960_test_gesture();
	apds9960_deinit();
}

