/**
 * @file main.c
 * @author ElectroNick (nick@dsd.dev)
 * @brief Main file of DeepDeck, an open source Macropad based on ESP32
 * @version 0.2
 * @date 2022-12-08
 * @copyright Copyright (c) 2022
 * 
 * MIT License
 * Copyright (c) 2022
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 * DeepDeck, a product by DeepSea Developments.
 * More info on DeepDeck @ www.deepdeck.co
 * DeepseaDev.com
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

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
#include "hal_ble.h"

//Deepdeck functions
#include "matrix.h"
#include "keyboard_config.h"
#include "battery_monitor.h"
#include "nvs_funcs.h"
#include "nvs_keymaps.h"
#include "mqtt.h"

#include "esp_err.h"

#include "plugins.h"
#include "deepdeck_tasks.h"
#include "gesture_handles.h"


#define BASE_PRIORITY 5

//plugin functions
static config_data_t config;

// xSemaphore for i2C shared resource
SemaphoreHandle_t xSemaphore = NULL;

/**
 * @brief Main tasks of ESP32. This is a tasks with priority level 1.
 * 
 * This task init all the basic hardware and then init the tasks needed to run DeepDeck
 */
void app_main() 
{
	xSemaphore = xSemaphoreCreateBinary();

	//Reset the rtc GPIOS
	rtc_matrix_deinit();
	// Setup keys matrix
	matrix_setup();
	
	// Initialize NVS (non volatile storage).
	esp_err_t ret;
	ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES) // If no space available, erase NVS and init it again
	{
		ESP_ERROR_CHECK (nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	// Read config from NVS
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


	// Set log level of the progam
	esp_log_level_set("*", ESP_LOG_INFO);

	//Loading layouts from nvs (if found)
	nvs_load_layouts();

	//activate keyboard BT stack
	halBLEInit(1, 1, 1, 0);
	ESP_LOGI("HIDD", "MAIN finished...");

	// init i2c
	int i2c_master_port = I2C_MASTER_NUM;
	i2c_config_t conf = { .mode = I2C_MODE_MASTER, .sda_io_num =
	I2C_MASTER_SDA_IO, .sda_pullup_en = GPIO_PULLUP_ENABLE,
			.scl_io_num = I2C_MASTER_SCL_IO, .scl_pullup_en =
					GPIO_PULLUP_ENABLE, .master.clk_speed =
			I2C_MASTER_FREQ_HZ, };
	i2c_bus_handle_t i2c_bus = i2c_bus_create(i2c_master_port, &conf);

	//activate gesture
#ifdef GESTURE_ENABLE
	apds9960_init(&i2c_bus);
	vTaskDelay(pdMS_TO_TICKS(200));
	xTaskCreate(gesture_task, " gesture task", 4096, NULL, (BASE_PRIORITY + 1),	&xGesture);
	ESP_LOGI("Gesture", "initialized");
#endif

	//activate oled
#ifdef	OLED_ENABLE
	init_oled(ROTATION);
	
	splashScreen();
	vTaskDelay(pdMS_TO_TICKS(1000));

	xTaskCreate(oled_task, "oled task", 1024*4, NULL,
			BASE_PRIORITY, &xOledTask);
	ESP_LOGI("Oled", "initialized");
#endif

	//activate encoder functions
#ifdef	R_ENCODER_1
	xTaskCreate(encoder_report, "encoder report", 4096, NULL,
			BASE_PRIORITY, NULL);
	ESP_LOGI("Encoder 1", "initialized");
#endif

#ifdef RGB_LEDS
	xTaskCreate(rgb_leds_task, "rgb_leds_task", 4096, NULL,
			BASE_PRIORITY, NULL);
	ESP_LOGI("rgb_leds_task", "initialized");
#endif

	// Start the keyboard Tasks
	// Create the key scanning task on core 1 (otherwise it will crash)
#ifdef MASTER
	BLE_EN = 1;
	xTaskCreate(key_reports, "key report task", 8192,
			xKeyreportTask, BASE_PRIORITY, NULL);
	ESP_LOGI("Keyboard task", "initialized");
#endif

#ifdef BATT_STAT
	init_batt_monitor();
		xTaskCreate(battery_reports, "battery reporst", 4096, NULL,
			BASE_PRIORITY, NULL);
	ESP_LOGI("Battery monitor", "initialized");
#endif

#ifdef SLEEP_MINS
	xTaskCreate(deep_sleep, "deep sleep task", 4096, NULL,
			BASE_PRIORITY, NULL);
	ESP_LOGI("Sleep", "initialized");
#endif


	ESP_LOGI("Main", "Main sequence done!");
	ESP_LOGI("Main", "Size of the dd_layer: %d bytes", sizeof(dd_layer));
}



