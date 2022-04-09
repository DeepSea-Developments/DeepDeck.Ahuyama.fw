/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * Copyright 2018 Gal Zaidenstein.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "driver/touch_pad.h"
#include "esp_timer.h"
#include "esp_sleep.h"
#include "esp_pm.h"

//HID Ble functions
//#include "HID_kbdmousejoystick.h"
#include "hal_ble.h"

//MK32 functions
#include "matrix.h"
#include "keypress_handles.c"
#include "keyboard_config.h"
#include "espnow_receive.h"
#include "espnow_send.h"
#include "r_encoder.h"
#include "battery_monitor.h"
#include "nvs_funcs.h"
#include "nvs_keymaps.h"

#define KEY_REPORT_TAG "KEY_REPORT"
#define SYSTEM_REPORT_TAG "KEY_REPORT"
#define TRUNC_SIZE 20
#define USEC_TO_SEC 1000000
#define SEC_TO_MIN 60
//plugin functions
#include "plugins.h"

static config_data_t config;
QueueHandle_t espnow_recieve_q;

bool DEEP_SLEEP = true; // flag to check if we need to go to deep sleep

#ifdef OLED_ENABLE
TaskHandle_t xOledTask;
#endif
TaskHandle_t xKeyreportTask;

//Task for continually updating the OLED
extern "C" void oled_task(void *pvParameters) {
#ifdef MASTER
	ble_connected_oled();
	bool CON_LOG_FLAG = false; // Just because I don't want it to keep logging the same thing a billion times
	while (1) {
		if (halBLEIsConnected() == 0) {
			if (CON_LOG_FLAG == false) {
				ESP_LOGI(KEY_REPORT_TAG,
						"Not connected, waiting for connection ");
			}
			waiting_oled();
			DEEP_SLEEP = false;
			CON_LOG_FLAG = true;
		} else {
			if (CON_LOG_FLAG == true) {
				ble_connected_oled();
			}
			update_oled();
			CON_LOG_FLAG = false;
		}
	}
#endif
#ifdef SLAVE
	while(1) {
		ble_slave_oled();
	}
#endif
}

//handle battery reports over BLE
extern "C" void battery_reports(void *pvParameters) {
	//uint8_t past_battery_report[1] = { 0 };

	while(1){
		uint32_t bat_level = get_battery_level();
		//if battery level is above 100, we're charging
		if(bat_level > 100){
			bat_level = 100;
			//if charging, do not enter deepsleep
			DEEP_SLEEP = false;
		}
		void* pReport = (void*) &bat_level;

		ESP_LOGI("Battery Monitor","battery level %d", bat_level);
		if(BLE_EN == 1){
			xQueueSend(battery_q, pReport, (TickType_t) 0);
		}
		if(input_str_q != NULL){
			xQueueSend(input_str_q, pReport, (TickType_t) 0);
		}
		vTaskDelay(60*1000/ portTICK_PERIOD_MS);
	}
}


//How to handle key reports
extern "C" void key_reports(void *pvParameters) {
	// Arrays for holding the report at various stages
	uint8_t past_report[REPORT_LEN] = { 0 };
	uint8_t report_state[REPORT_LEN];



	while (1) {
		memcpy(report_state, check_key_state(layouts[current_layout]),
				sizeof report_state);

		//Do not send anything if queues are uninitialized
		if (mouse_q == NULL || keyboard_q == NULL || joystick_q == NULL) {
			ESP_LOGE(KEY_REPORT_TAG, "queues not initialized");
			continue;
		}

		//Check if the report was modified, if so send it
		if (memcmp(past_report, report_state, sizeof past_report) != 0) {
			DEEP_SLEEP = false;
			void* pReport;
			memcpy(past_report, report_state, sizeof past_report);

#ifndef NKRO
			uint8_t trunc_report[REPORT_LEN] = {0};
			trunc_report[0] = report_state[0];
			trunc_report[1] = report_state[1];

			uint16_t cur_index = 2;
			//Phone's mtu size is usuaully limited to 20 bytes
			for(uint16_t i = 2; i < REPORT_LEN && cur_index < TRUNC_SIZE; ++i){
				if(report_state[i] != 0){
					trunc_report[cur_index] = report_state[i];
					++cur_index;
				}
			}

			pReport = (void *) &trunc_report;
#endif
#ifdef NKRO
			pReport = (void *) &report_state;
#endif

			if(BLE_EN == 1){
				xQueueSend(keyboard_q, pReport, (TickType_t) 0);
			}
			if(input_str_q != NULL){
				xQueueSend(input_str_q, pReport, (TickType_t) 0);
			}
		}

	}

}

//Handling rotary encoder
extern "C" void encoder_report(void *pvParameters) {
	uint8_t encoder_state = 0;
	uint8_t past_encoder_state = 0;

	while (1) {
		encoder_state = r_encoder_state();
		if (encoder_state != past_encoder_state) {
			DEEP_SLEEP = false;
			r_encoder_command(encoder_state, encoder_map[current_layout]);
			past_encoder_state = encoder_state;
		}

	}
}

//Handling rotary encoder for slave pad
extern "C" void slave_encoder_report(void *pvParameters) {
	uint8_t encoder_state = 0;
	uint8_t past_encoder_state = 0;

	while (1) {
		encoder_state = r_encoder_state();
		if (encoder_state != past_encoder_state) {
			DEEP_SLEEP = false;
			xQueueSend(espnow_encoder_send_q, (void*) &encoder_state,
					(TickType_t) 0);
			past_encoder_state = encoder_state;
		}

	}
}

//Function for sending out the modified matrix
extern "C" void slave_scan(void *pvParameters) {

	uint8_t PAST_MATRIX[MATRIX_ROWS][MATRIX_COLS] = { 0 };

	while (1) {
		scan_matrix();
		if (memcmp(&PAST_MATRIX, &MATRIX_STATE, sizeof MATRIX_STATE) != 0) {
			DEEP_SLEEP = false;
			memcpy(&PAST_MATRIX, &MATRIX_STATE, sizeof MATRIX_STATE);
			xQueueSend(espnow_matrix_send_q, (void*) &MATRIX_STATE,
					(TickType_t) 0);

		}
	}
}

//Update the matrix state via reports recieved by espnow
extern "C" void espnow_update_matrix(void *pvParameters) {

	uint8_t CURRENT_MATRIX[MATRIX_ROWS][MATRIX_COLS] = { 0 };
	while (1) {
		if (xQueueReceive(espnow_receive_q, &CURRENT_MATRIX, 10000)) {
			DEEP_SLEEP = false;
			memcpy(&SLAVE_MATRIX_STATE, &CURRENT_MATRIX, sizeof CURRENT_MATRIX);
		}
	}
}
//what to do after waking from deep sleep, doesn't seem to work after updating esp-idf
//extern "C" void RTC_IRAM_ATTR esp_wake_deep_sleep(void) {
//    rtc_matrix_deinit();;
//    SLEEP_WAKE=true;
//}

/*If no key press has been recieved in SLEEP_MINS amount of minutes, put device into deep sleep
 *  wake up on touch on GPIO pin 2
 *  */
#ifdef SLEEP_MINS
extern "C" void deep_sleep(void *pvParameters) {
	uint64_t initial_time = esp_timer_get_time(); // notice that timer returns time passed in microseconds!
	uint64_t current_time_passed = 0;
	while (1) {

		current_time_passed = (esp_timer_get_time() - initial_time);

		if (DEEP_SLEEP == false) {
			current_time_passed = 0;
			initial_time = esp_timer_get_time();
			DEEP_SLEEP = true;
		}

		if (((double)current_time_passed/USEC_TO_SEC) >= (double)  (SEC_TO_MIN * SLEEP_MINS)) {
			if (DEEP_SLEEP == true) {
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
			if (DEEP_SLEEP == false) {
				current_time_passed = 0;
				initial_time = esp_timer_get_time();
				DEEP_SLEEP = true;
			}
		}

	}

}
#endif

extern "C" void app_main() {
	//Reset the rtc GPIOS
	rtc_matrix_deinit();

	//Underclocking for better current draw (not really effective)
	//	esp_pm_config_esp32_t pm_config;
	//	pm_config.max_freq_mhz = 10;
	//	pm_config.min_freq_mhz = 10;
	//	esp_pm_configure(&pm_config);
	matrix_setup();
	esp_err_t ret;

	// Initialize NVS.
	ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
		ESP_ERROR_CHECK (nvs_flash_erase());ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	// Read config
	nvs_handle my_handle;
	ESP_LOGI("MAIN", "loading configuration from NVS");
	ret = nvs_open("config_c", NVS_READWRITE, &my_handle);
	if (ret != ESP_OK)
		ESP_LOGE("MAIN", "error opening NVS");
	size_t available_size = MAX_BT_DEVICENAME_LENGTH;
	strcpy(config.bt_device_name, GATTS_TAG);
	nvs_get_str(my_handle, "btname", config.bt_device_name, &available_size);
	if (ret != ESP_OK) {
		ESP_LOGE("MAIN", "error reading NVS - bt name, setting to default");
		strcpy(config.bt_device_name, GATTS_TAG);
	} else
		ESP_LOGI("MAIN", "bt device name is: %s", config.bt_device_name);

	esp_log_level_set("*", ESP_LOG_INFO);

	//Loading layouts from nvs (if found)
#ifdef MASTER
	nvs_load_layouts();
	//activate keyboard BT stack
	halBLEInit(1, 1, 1, 0);
	ESP_LOGI("HIDD", "MAIN finished...");
#endif

	//If the device is a slave initialize sending reports to master
#ifdef SLAVE
	xTaskCreatePinnedToCore(slave_scan, "Scan matrix changes for slave", 4096, xKeyreportTask, configMAX_PRIORITIES, NULL,1);
#ifdef R_ENCODER_SLAVE
	xTaskCreatePinnedToCore(slave_encoder_report, "Scan encoder changes for slave", 4096, NULL, configMAX_PRIORITIES, NULL,1);
#endif
	espnow_send();
#endif

	//If the device is a master for split board initialize receiving reports from slave
#ifdef SPLIT_MASTER
	espnow_receive_q = xQueueCreate(32, REPORT_LEN * sizeof(uint8_t));
	espnow_receive();
	xTaskCreatePinnedToCore(espnow_update_matrix, "ESP-NOW slave matrix state",
			4096, NULL, configMAX_PRIORITIES, NULL, 1);
	ESP_LOGI("ESPNOW", "initializezd");

#endif

	//activate encoder functions
#ifdef	R_ENCODER
	r_encoder_setup();
	xTaskCreatePinnedToCore(encoder_report, "encoder report", 4096, NULL,
			configMAX_PRIORITIES, NULL, 1);
	ESP_LOGI("Encoder", "initializezd");
#endif

	// Start the keyboard Tasks
	// Create the key scanning task on core 1 (otherwise it will crash)
#ifdef MASTER
	BLE_EN = 1;
	xTaskCreatePinnedToCore(key_reports, "key report task", 8192,
			xKeyreportTask, configMAX_PRIORITIES, NULL, 1);
	ESP_LOGI("Keyboard task", "initializezd");
#endif
	//activate oled
#ifdef	OLED_ENABLE
	init_oled(ROTATION);
	xTaskCreatePinnedToCore(oled_task, "oled task", 4096, NULL,
			configMAX_PRIORITIES, &xOledTask, 1);
	ESP_LOGI("Oled", "initializezd");
#endif

#ifdef BATT_STAT
	init_batt_monitor();
		xTaskCreatePinnedToCore(battery_reports, "battery reporst", 4096, NULL,
			configMAX_PRIORITIES, NULL, 1);
	ESP_LOGI("Battery monitor", "initializezd");
#endif

#ifdef SLEEP_MINS
	xTaskCreatePinnedToCore(deep_sleep, "deep sleep task", 4096, NULL,
			configMAX_PRIORITIES, NULL, 1);
	ESP_LOGI("Sleep", "initializezd");
#endif

//This is for testing
	//init_layout_server();
	//input_string();
}
}
