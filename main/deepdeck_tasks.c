#include "deepdeck_tasks.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

//MK32 functions
// #include "matrix.h"
#include "keypress_handles.c"
// #include "keyboard_config.h"
#include "battery_monitor.h"
#include "nvs_funcs.h"
// #include "nvs_keymaps.h"
// #include "mqtt.h"

#include "esp_err.h"

// #include "rotary_encoder.h"
#include "rgb_led.h"
#include "menu.h"

//HID Ble functions
#include "hal_ble.h"

#include "keycode_conv.h"

static const char * TAG = "KeyReport";

//Task for continually updating the OLED
// ToDo: Add a better way to handle freertos task without running wild.
void oled_task(void *pvParameters) 
{
	ble_connected_oled();
	bool CON_LOG_FLAG = false; // Just because I don't want it to keep logging the same thing a billion times
	while (1) 
	{
		switch(deepdeck_status)
		{
			case S_NORMAL:
				if (halBLEIsConnected() == 0) {
					if (CON_LOG_FLAG == false) {
						ESP_LOGI(TAG,
								"Not connected, waiting for connection ");
					}
					waiting_oled();
					//DEEP_SLEEP = false;
					CON_LOG_FLAG = true;
				} else {
					if (CON_LOG_FLAG == true) {
						ble_connected_oled();
					}
					update_oled();
					CON_LOG_FLAG = false;
				}
			break;
			case S_SETTINGS:
				menu_init();
				vTaskDelay(pdMS_TO_TICKS(200));

				menu_screen();
				ble_connected_oled();

				deepdeck_status = S_NORMAL;

			break;
		}
		vTaskDelay(pdMS_TO_TICKS(50));
	}

}


//handle battery reports over BLE
void battery_reports(void *pvParameters) 
{
	//uint8_t past_battery_report[1] = { 0 };

	while(1){
		uint32_t bat_level = get_battery_level();
		//if battery level is above 100, we're charging
		if(bat_level > 100){
			bat_level = 100;
			//if charging, do not enter deepsleep
			// DEEP_SLEEP = false;
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
void key_reports(void *pvParameters)
{
	// Arrays for holding the report at various stages
	uint8_t past_report[REPORT_LEN] = { 0 };
	uint8_t report_state[REPORT_LEN];

	while (1) 
	{
		memcpy(report_state, check_key_state(layouts[current_layout]),
				sizeof report_state);

		//Do not send anything if queues are uninitialized
		if (mouse_q == NULL || keyboard_q == NULL || joystick_q == NULL) {
			ESP_LOGE(TAG, "queues not initialized");
			continue;
		}

		//Check if the report was modified, if so send it
		if (memcmp(past_report, report_state, sizeof past_report) != 0) {
			// DEEP_SLEEP = false;
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
		vTaskDelay(pdMS_TO_TICKS(10));
	}

}

//Handling rgb LEDs
void rgb_leds_task(void *pvParameters) 
{
	
	rgb_key_led_init();
	rgb_notification_led_init();
	while (1) {
		key_led_modes();
		taskYIELD();
	}
}