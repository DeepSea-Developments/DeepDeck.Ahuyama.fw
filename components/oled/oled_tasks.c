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

#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <assert.h>

#include "oled_tasks.h"

#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"

#include "u8g2_esp32_hal.h"
#include "keyboard_config.h"
#include "battery_monitor.h"
#include "nvs_keymaps.h"

static const char *TAG = "	OLED";

u8g2_t u8g2; // a structure which will contain all the data for one display
uint8_t prev_led = 0;

QueueHandle_t layer_recieve_q;
QueueHandle_t led_recieve_q;

uint32_t battery_percent = 0;
uint32_t prev_battery_percent = 0;

uint8_t curr_layout = 0;
uint8_t current_led = 0;

int BATT_FLAG = 0;
int DROP_H = 0;

int offset_x_batt = 0;
int offset_y_batt = 0;

char current_ip[16] = "...";

#define BT_ICON 0x5e
#define BATT_ICON 0x5b
#define LOCK_ICON 0xca

// Erasing area from oled
void erase_area(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
	u8g2_SetDrawColor(&u8g2, 0);
	u8g2_DrawBox(&u8g2, x, y, w, h);
	u8g2_SetDrawColor(&u8g2, 1);
}

// Function for updating the OLED
void update_oled(void)
{
#ifdef BATT_STAT
	battery_percent = get_battery_level();
#endif

	if (xQueueReceive(layer_recieve_q, &curr_layout, (TickType_t)0))
	{

		erase_area(0, 13, 128, 22);
		u8g2_SetFont(&u8g2, u8g2_font_courB18_tf);
		u8g2_DrawStr(&u8g2, 0, 33, g_user_layers[curr_layout].name);

		erase_area(0, 35, 128, 29);
		u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
		for (int i = 0; i < MATRIX_COLS; i++)
		{
			for (int j = 0; j < MATRIX_ROWS; j++)
			{
				u8g2_DrawStr(&u8g2, j * 32, 42 + i * 7, g_user_layers[curr_layout].key_map_names[i][j]);
			}
		}

		u8g2_SendBuffer(&u8g2);
	}
	if (xQueueReceive(led_recieve_q, &current_led, (TickType_t)0))
	{
		erase_area(0, 24, 127, 8);
		if (CHECK_BIT(current_led, 0) != 0)
		{
			u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
			u8g2_DrawStr(&u8g2, 0, 31, "NUM");
			u8g2_SetFont(&u8g2, u8g2_font_open_iconic_all_1x_t);
			u8g2_DrawGlyph(&u8g2, 16, 32, LOCK_ICON);
		}

		if (CHECK_BIT(current_led, 1) != 0)
		{
			u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
			u8g2_DrawStr(&u8g2, 27, 31, "CAPS");
			u8g2_SetFont(&u8g2, u8g2_font_open_iconic_all_1x_t);
			u8g2_DrawGlyph(&u8g2, 48, 32, LOCK_ICON);
		}
		if (CHECK_BIT(current_led, 2) != 0)
		{
			u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
			u8g2_DrawStr(&u8g2, 57, 31, "SCROLL");
			u8g2_SetFont(&u8g2, u8g2_font_open_iconic_all_1x_t);
			u8g2_DrawGlyph(&u8g2, 88, 32, LOCK_ICON);
		}
		u8g2_SendBuffer(&u8g2);
	}

	if (battery_percent != prev_battery_percent)
	{
		u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
		char buf[sizeof(uint32_t)];
		snprintf(buf, sizeof(uint32_t), "%lu", battery_percent);
		u8g2_DrawStr(&u8g2, 103 + offset_x_batt, 7 + offset_y_batt, "%");
		if ((battery_percent < 100) && (abs((int)battery_percent - (int)prev_battery_percent) >= 2))
		{
			erase_area(85 + offset_x_batt, 0 + offset_y_batt, 15, 7);
			u8g2_DrawStr(&u8g2, 90 + offset_x_batt, 7 + offset_y_batt, buf);
			u8g2_SendBuffer(&u8g2);
		}
		if ((battery_percent > 100) && (BATT_FLAG = 0))
		{
			erase_area(85 + offset_x_batt, 0 + offset_y_batt, 15, 7);
			u8g2_DrawStr(&u8g2, 85 + offset_x_batt, 7 + offset_y_batt, "100");
			BATT_FLAG = 1;
			u8g2_SendBuffer(&u8g2);
		}
		if (battery_percent == 100)
		{
			erase_area(85 + offset_x_batt, 0 + offset_y_batt, 15, 7);
			u8g2_DrawStr(&u8g2, 85 + offset_x_batt, 7 + offset_y_batt, "100");
			u8g2_SendBuffer(&u8g2);
		}
		prev_battery_percent = battery_percent;
	}
}

// oled on connection
void ble_connected_oled(void)
{

	u8g2_ClearBuffer(&u8g2);
	u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
	u8g2_DrawStr(&u8g2, 0, 6, GATTS_TAG);

	// u8g2_DrawStr(&u8g2, 0, 14, layer_names_arr[current_layout]);
	// u8g2_SetFont(&u8g2, u8g2_font_open_iconic_all_1x_t);
	// u8g2_DrawGlyph(&u8g2, 110 + offset_x_batt, 8 + offset_y_batt, BATT_ICON);
	// u8g2_DrawGlyph(&u8g2, 120 + offset_x_batt, 8 + offset_y_batt, BT_ICON);

	//Print Wifi status
	u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
	u8g2_DrawStr(&u8g2, 40 + offset_x_batt, 8 + offset_y_batt, current_ip);

	u8g2_SetFont(&u8g2, u8g2_font_courB18_tf);
	u8g2_DrawStr(&u8g2, 0, 33, g_user_layers[curr_layout].name);

	u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
	for (int i = 0; i < MATRIX_COLS; i++)
	{
		for (int j = 0; j < MATRIX_ROWS; j++)
		{
			u8g2_DrawStr(&u8g2, j * 32, 42 + i * 7, g_user_layers[curr_layout].key_map_names[i][j]);
		}
	}

	//	if(CHECK_BIT(curr_led,0)!=0){
	//		u8g2_SetFont(&u8g2, u8g2_font_5x7_tf );
	//		u8g2_DrawStr(&u8g2, 0,31,"NUM");
	//		u8g2_SetFont(&u8g2, u8g2_font_open_iconic_all_1x_t );
	//		u8g2_DrawGlyph(&u8g2, 16,32,LOCK_ICON);
	//	}
	//
	//	if(CHECK_BIT(curr_led,1)!=0){
	//		u8g2_SetFont(&u8g2, u8g2_font_5x7_tf );
	//		u8g2_DrawStr(&u8g2, 27,31,"CAPS");
	//		u8g2_SetFont(&u8g2, u8g2_font_open_iconic_all_1x_t );
	//		u8g2_DrawGlyph(&u8g2,48,32,LOCK_ICON);
	//	}
	//	if(CHECK_BIT(curr_led,2)!=0){
	//		u8g2_SetFont(&u8g2, u8g2_font_5x7_tf );
	//		u8g2_DrawStr(&u8g2, 57,31,"SCROLL");
	//		u8g2_SetFont(&u8g2, u8g2_font_open_iconic_all_1x_t );
	//		u8g2_DrawGlyph(&u8g2,88,32,LOCK_ICON);
	//	}

	u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
	char buf[sizeof(uint32_t)];
	snprintf(buf, sizeof(uint32_t), "%lu", battery_percent);
	u8g2_DrawStr(&u8g2, +offset_x_batt, +offset_y_batt, "%");
	if (battery_percent < 100)
	{
		u8g2_DrawStr(&u8g2, +offset_x_batt, 7 + offset_y_batt, buf);
	}
	else
	{
		u8g2_DrawStr(&u8g2, 85 + offset_x_batt, 7 + offset_y_batt, "100");
	}
	u8g2_SendBuffer(&u8g2);
}

void wifi_connected_oled(char *ip_char)
{
	//u8g2_ClearBuffer(&u8g2);
	strcpy(current_ip,ip_char);

	u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
	u8g2_DrawStr(&u8g2, 40 + offset_x_batt, 8 + offset_y_batt, ip_char);
	//u8g2_SendBuffer(&u8g2); ------> This makes system crash if bluetooth is not connected at the beginning of the system
}

// Waiting for connecting animation
void waiting_oled(void)
{
	char waiting[] = "Waiting for ";
	char conn[] = "connection";

#ifdef BATT_STAT
	battery_percent = get_battery_level();
#endif

	u8g2_ClearBuffer(&u8g2);
	// u8g2_SetFont(&u8g2, u8g2_font_open_iconic_all_1x_t);
	// u8g2_DrawGlyph(&u8g2, 110 + offset_x_batt, 8 + offset_y_batt, BATT_ICON);
	// u8g2_DrawGlyph(&u8g2, 120 + offset_x_batt, 8 + offset_y_batt, BT_ICON);
	u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
	u8g2_DrawStr(&u8g2, 0, 6, GATTS_TAG);

	// char buf[sizeof(uint32_t)];
	// snprintf(buf, sizeof(uint32_t), "%d", battery_percent);
	// u8g2_DrawStr(&u8g2, 103 + offset_x_batt, 7 + offset_y_batt, "%");
	// if ((battery_percent < 100) && (battery_percent - prev_battery_percent >= 2))
	// {
	// 	u8g2_DrawStr(&u8g2, 90 + offset_x_batt, 7 + offset_y_batt, buf);
	// }
	// if (battery_percent < 100)
	// {
	// 	u8g2_DrawStr(&u8g2, 90 + offset_x_batt, 7 + offset_y_batt, buf);
	// }
	// else
	// {
	// 	u8g2_DrawStr(&u8g2, 85 + offset_x_batt, 7 + offset_y_batt, "100");
	// }

	u8g2_DrawStr(&u8g2, 0, 26, waiting);
	u8g2_DrawStr(&u8g2, 0, 40, conn);
	u8g2_SendBuffer(&u8g2);

	// for (int i = 0; i < 3; i++)
	// {
	// 	u8g2_DrawStr(&u8g2, 0, 26, waiting);
	// 	u8g2_DrawStr(&u8g2, 0, 40, conn);
	// 	u8g2_SendBuffer(&u8g2);
	// 	vTaskDelay(100 / portTICK_PERIOD_MS);
	// 	strcat(conn, ".");
	// }
}

// shut down OLED
void deinit_oled(void)
{

	u8g2_ClearBuffer(&u8g2);
	u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
	u8g2_DrawStr(&u8g2, 0, 6, GATTS_TAG);
	u8g2_DrawStr(&u8g2, 0, 26, "Going to sleep!");
	u8g2_SendBuffer(&u8g2);
	vTaskDelay(1000 / portTICK_PERIOD_MS);
	u8g2_ClearDisplay(&u8g2);
	u8g2_SetPowerSave(&u8g2, 1);
	i2c_driver_delete(I2C_NUM_0);
}

// initialize oled
void init_oled(const u8g2_cb_t *rotation)
{

	layer_recieve_q = xQueueCreate(32, sizeof(uint8_t));
	led_recieve_q = xQueueCreate(32, sizeof(uint8_t));
	ESP_LOGI(TAG, "Setting up oled");

	u8g2_esp32_hal_t u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT;
	u8g2_esp32_hal.sda = OLED_SDA_PIN;
	u8g2_esp32_hal.scl = OLED_SCL_PIN;
	u8g2_esp32_hal_init(u8g2_esp32_hal);

	if ((rotation == DEG90) || rotation == DEG270)
	{

		offset_x_batt = -85;
		offset_y_batt = 120;
	}

	u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2, rotation,
										   u8g2_esp32_i2c_byte_cb, u8g2_esp32_gpio_and_delay_cb); // init u8g2 structure

	u8x8_SetI2CAddress(&u8g2.u8x8, 0x78);
	u8g2_InitDisplay(&u8g2);	 // send init sequence to the display, display is in sleep mode after this,
	u8g2_SetPowerSave(&u8g2, 0); // wake up display

	u8g2_ClearBuffer(&u8g2);
}

/**
 * https://marlinfw.org/tools/u8glib/converter.html
 * Made with Marlin Bitmap Converter
 *
 * This bitmap from the file 'DeepDeck.png'
 */

// const unsigned char bitmap_splash[] = {
// 	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
// 	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
// 	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xDF,
// 	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x8F,
// 	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x0F,
// 	0xFF, 0xFF, 0xF0, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE0, 0x1F, 0xFF, 0xFF, 0xFF, 0x1F,
// 	0xFF, 0xF8, 0x00, 0x03, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0x00, 0x07, 0xFF, 0xFF, 0xFE, 0x1F,
// 	0xFE, 0x00, 0x00, 0x03, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0x00, 0x00, 0x07, 0xFF, 0xFF, 0xFE, 0x3F,
// 	0xF8, 0x00, 0x1F, 0x03, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0x00, 0x3E, 0x07, 0xFF, 0xFF, 0xFC, 0x3F,
// 	0xC0, 0x0F, 0xFE, 0x03, 0xFF, 0xFF, 0xFF, 0xFF, 0x80, 0x1F, 0xFC, 0x07, 0xFF, 0xFF, 0xFC, 0x7F,
// 	0x80, 0x7F, 0xFC, 0x07, 0x9F, 0xFC, 0xFF, 0xFF, 0x00, 0xFF, 0xF8, 0x0F, 0x3F, 0xFE, 0xF8, 0xFF,
// 	0xF0, 0x7F, 0xF0, 0x0E, 0x0F, 0xF0, 0x7F, 0xFF, 0xE0, 0xFF, 0xE0, 0x1C, 0x1F, 0xFC, 0x70, 0xFF,
// 	0xF0, 0xFF, 0xE0, 0x18, 0x0F, 0xC0, 0x7E, 0x00, 0xE1, 0xFF, 0xC0, 0x30, 0x1F, 0xF0, 0x71, 0xFF,
// 	0xF0, 0xFF, 0x80, 0x70, 0x8F, 0x84, 0x78, 0x00, 0x61, 0xFF, 0x00, 0xE1, 0x1F, 0xC0, 0x61, 0xC3,
// 	0xE0, 0xFE, 0x01, 0xE3, 0x1F, 0x18, 0xE0, 0x78, 0x41, 0xFC, 0x03, 0xC6, 0x3F, 0x80, 0x60, 0x07,
// 	0xE1, 0xFC, 0x07, 0xC2, 0x3E, 0x11, 0xE0, 0xF0, 0x43, 0xF8, 0x0F, 0x84, 0x7E, 0x00, 0xE0, 0x1F,
// 	0xE1, 0xF0, 0x1F, 0x80, 0xF8, 0x07, 0xF8, 0xF0, 0xC3, 0xE0, 0x3F, 0x01, 0xFC, 0x1C, 0xC0, 0x7F,
// 	0xE3, 0xE0, 0x3F, 0x03, 0xC0, 0x1E, 0x31, 0xE1, 0xC7, 0xC0, 0x7E, 0x07, 0x88, 0x3F, 0xC0, 0xFF,
// 	0xC3, 0x80, 0xFF, 0x3E, 0x01, 0xF0, 0x71, 0xC3, 0x87, 0x01, 0xFE, 0x7C, 0x10, 0x7F, 0x80, 0x7F,
// 	0xC7, 0x03, 0xFE, 0x38, 0x31, 0xC1, 0xE1, 0x87, 0x8E, 0x07, 0xFC, 0x70, 0x70, 0xFE, 0x08, 0x0F,
// 	0xC4, 0x0F, 0xFF, 0x00, 0x78, 0x03, 0xE3, 0x0F, 0x88, 0x1F, 0xFE, 0x00, 0xF1, 0xE1, 0x8F, 0x8F,
// 	0xE8, 0x1F, 0xFF, 0x01, 0xF8, 0x0F, 0xC2, 0x3F, 0xD0, 0x3F, 0xFE, 0x03, 0xF0, 0x07, 0x1F, 0xFF,
// 	0xF0, 0x7F, 0xFF, 0x87, 0xFC, 0x3F, 0xC4, 0x7F, 0xE0, 0xFF, 0xFF, 0x0F, 0xF0, 0x1F, 0x1F, 0xFF,
// 	0xE1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC7, 0xFF, 0xC3, 0xFF, 0xFF, 0xFF, 0xF8, 0x7F, 0x3F, 0xFF,
// 	0xC7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x8F, 0xFF, 0x8F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3F, 0xFF,
// 	0xBF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x8F, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
// 	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x9F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
// 	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x1F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
// 	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
// 	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
// 	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
// 	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
// 	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
// 	0xFF, 0xFF, 0xFF, 0xFF, 0xC7, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
// 	0xFF, 0xFF, 0xFF, 0xFF, 0x87, 0xFC, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
// 	0xFF, 0xFF, 0xFF, 0xFF, 0x87, 0xF8, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
// 	0xFF, 0xFF, 0xFF, 0xFF, 0x07, 0xF8, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
// 	0xFF, 0xFF, 0xFF, 0xFE, 0x07, 0xF1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
// 	0xFF, 0xFF, 0xFF, 0xFC, 0x07, 0xE1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
// 	0xFF, 0xFF, 0xFF, 0xFC, 0x07, 0xC3, 0xFF, 0xFF, 0xFD, 0xFE, 0x3F, 0xFF, 0x7F, 0xF1, 0xFF, 0xFF,
// 	0xFF, 0xFF, 0xFF, 0xF8, 0x0F, 0xC7, 0xFF, 0xFF, 0xF8, 0x7E, 0x3F, 0xFC, 0x3F, 0xF1, 0xFF, 0xFF,
// 	0xFF, 0xFF, 0xFF, 0xF0, 0x8F, 0x8F, 0xFF, 0xDC, 0xF0, 0xC0, 0x3F, 0x38, 0x3E, 0x01, 0xFF, 0xFF,
// 	0xFF, 0xFF, 0xFF, 0xF1, 0x8F, 0x0C, 0xF3, 0x8C, 0xE0, 0x00, 0x3C, 0x30, 0x78, 0x01, 0xFF, 0xFF,
// 	0xFF, 0xFF, 0xFF, 0xE1, 0x8F, 0x00, 0x23, 0x18, 0xC0, 0x00, 0x38, 0x00, 0x70, 0x01, 0xFF, 0xFF,
// 	0xFF, 0xFF, 0xFF, 0xC3, 0x8E, 0x00, 0x23, 0x19, 0x80, 0x0C, 0x70, 0x00, 0x70, 0x63, 0xFF, 0xFF,
// 	0xFF, 0xFF, 0xFF, 0x82, 0x02, 0x00, 0x46, 0x11, 0x00, 0x38, 0x70, 0x04, 0xE1, 0xC3, 0xFF, 0xFF,
// 	0xFF, 0xFF, 0xFF, 0x80, 0x00, 0x00, 0x4C, 0x30, 0x08, 0x60, 0x60, 0x08, 0xC3, 0x03, 0xFF, 0xFF,
// 	0xFF, 0xFF, 0xFE, 0x00, 0x00, 0x78, 0x80, 0x30, 0x18, 0xC0, 0xC3, 0x39, 0xC6, 0x07, 0xFF, 0xFF,
// 	0xFF, 0xFF, 0xF8, 0x00, 0x1C, 0xF8, 0x80, 0x30, 0x1C, 0x08, 0xC7, 0xF9, 0xE0, 0x47, 0xFF, 0xFF,
// 	0xFF, 0xFF, 0xC0, 0x3F, 0xB9, 0xF9, 0x82, 0x7C, 0x3C, 0x1C, 0xCF, 0xF9, 0xE0, 0xE7, 0xFF, 0xFF,
// 	0xFF, 0xFF, 0xF8, 0x7F, 0x39, 0xF9, 0x87, 0x7C, 0x7E, 0x7C, 0xFF, 0xF9, 0xF3, 0xE7, 0xFF, 0xFF,
// 	0xFF, 0xFF, 0xF8, 0xFF, 0xFF, 0xFD, 0xDF, 0xF8, 0xFF, 0xFF, 0xFF, 0xFD, 0xFF, 0xFF, 0xFF, 0xFF,
// 	0xFF, 0xFF, 0xF1, 0xFF, 0xFF, 0xFF, 0xFF, 0xF8, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
// 	0xFF, 0xFF, 0xFB, 0xFF, 0xFF, 0xFF, 0xFF, 0xF1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
// 	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF3, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
// 	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
// 	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
// 	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xCF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
// 	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xCF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
// 	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xDF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
// 	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
// 	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
// 	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
// 	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

/**
 * Made with Marlin Bitmap Converter
 * https://marlinfw.org/tools/u8glib/converter.html
 *
 * This bitmap from the file 'DeepDeck_1.png'
 */

const unsigned char bitmap_deepdeck[] = {
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x20,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x70,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF0,
  0x00,0x00,0x0F,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x1F,0xE0,0x00,0x00,0x00,0xE0,
  0x00,0x07,0xFF,0xFC,0x00,0x00,0x00,0x00,0x00,0x0F,0xFF,0xF8,0x00,0x00,0x01,0xE0,
  0x01,0xFF,0xFF,0xFC,0x00,0x00,0x00,0x00,0x03,0xFF,0xFF,0xF8,0x00,0x00,0x01,0xC0,
  0x07,0xFF,0xE0,0xFC,0x00,0x00,0x00,0x00,0x0F,0xFF,0xC1,0xF8,0x00,0x00,0x03,0xC0,
  0x3F,0xF0,0x01,0xFC,0x00,0x00,0x00,0x00,0x7F,0xE0,0x03,0xF8,0x00,0x00,0x03,0x80,
  0x7F,0x80,0x03,0xF8,0x60,0x03,0x00,0x00,0xFF,0x00,0x07,0xF0,0xC0,0x01,0x07,0x00,
  0x0F,0x80,0x0F,0xF1,0xF0,0x0F,0x80,0x00,0x1F,0x00,0x1F,0xE3,0xE0,0x03,0x8F,0x00,
  0x0F,0x00,0x1F,0xE7,0xF0,0x3F,0x81,0xFF,0x1E,0x00,0x3F,0xCF,0xE0,0x0F,0x8E,0x00,
  0x0F,0x00,0x7F,0x8F,0x70,0x7B,0x87,0xFF,0x9E,0x00,0xFF,0x1E,0xE0,0x3F,0x9E,0x3C,
  0x1F,0x01,0xFE,0x1C,0xE0,0xE7,0x1F,0x87,0xBE,0x03,0xFC,0x39,0xC0,0x7F,0x9F,0xF8,
  0x1E,0x03,0xF8,0x3D,0xC1,0xEE,0x1F,0x0F,0xBC,0x07,0xF0,0x7B,0x81,0xFF,0x1F,0xE0,
  0x1E,0x0F,0xE0,0x7F,0x07,0xF8,0x07,0x0F,0x3C,0x1F,0xC0,0xFE,0x03,0xE3,0x3F,0x80,
  0x1C,0x1F,0xC0,0xFC,0x3F,0xE1,0xCE,0x1E,0x38,0x3F,0x81,0xF8,0x77,0xC0,0x3F,0x00,
  0x3C,0x7F,0x00,0xC1,0xFE,0x0F,0x8E,0x3C,0x78,0xFE,0x01,0x83,0xEF,0x80,0x7F,0x80,
  0x38,0xFC,0x01,0xC7,0xCE,0x3E,0x1E,0x78,0x71,0xF8,0x03,0x8F,0x8F,0x01,0xF7,0xF0,
  0x3B,0xF0,0x00,0xFF,0x87,0xFC,0x1C,0xF0,0x77,0xE0,0x01,0xFF,0x0E,0x1E,0x70,0x70,
  0x17,0xE0,0x00,0xFE,0x07,0xF0,0x3D,0xC0,0x2F,0xC0,0x01,0xFC,0x0F,0xF8,0xE0,0x00,
  0x0F,0x80,0x00,0x78,0x03,0xC0,0x3B,0x80,0x1F,0x00,0x00,0xF0,0x0F,0xE0,0xE0,0x00,
  0x1E,0x00,0x00,0x00,0x00,0x00,0x38,0x00,0x3C,0x00,0x00,0x00,0x07,0x80,0xC0,0x00,
  0x38,0x00,0x00,0x00,0x00,0x00,0x70,0x00,0x70,0x00,0x00,0x00,0x00,0x00,0xC0,0x00,
  0x40,0x00,0x00,0x00,0x00,0x00,0x70,0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0xE0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};


/**
 * Made with Marlin Bitmap Converter
 * https://marlinfw.org/tools/u8glib/converter.html
 *
 * This bitmap from the file 'DeepDeck_2.png'
 */

const unsigned char bitmap_ahuyama[] = {
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x38,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x78,0x03,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x78,0x07,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0xF8,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x01,0xF8,0x0E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x03,0xF8,0x1E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x03,0xF8,0x3C,0x00,0x00,0x02,0x01,0xC0,0x00,0x80,0x0E,0x00,0x00,
  0x00,0x00,0x00,0x07,0xF0,0x38,0x00,0x00,0x07,0x81,0xC0,0x03,0xC0,0x0E,0x00,0x00,
  0x00,0x00,0x00,0x0F,0x70,0x70,0x00,0x23,0x0F,0x3F,0xC0,0xC7,0xC1,0xFE,0x00,0x00,
  0x00,0x00,0x00,0x0E,0x70,0xF3,0x0C,0x73,0x1F,0xFF,0xC3,0xCF,0x87,0xFE,0x00,0x00,
  0x00,0x00,0x00,0x1E,0x70,0xFF,0xDC,0xE7,0x3F,0xFF,0xC7,0xFF,0x8F,0xFE,0x00,0x00,
  0x00,0x00,0x00,0x3C,0x71,0xFF,0xDC,0xE6,0x7F,0xF3,0x8F,0xFF,0x8F,0x9C,0x00,0x00,
  0x00,0x00,0x00,0x7D,0xFD,0xFF,0xB9,0xEE,0xFF,0xC7,0x8F,0xFB,0x1E,0x3C,0x00,0x00,
  0x00,0x00,0x00,0x7F,0xFF,0xFF,0xB3,0xCF,0xF7,0x9F,0x9F,0xF7,0x3C,0xFC,0x00,0x00,
  0x00,0x00,0x01,0xFF,0xFF,0x87,0x7F,0xCF,0xE7,0x3F,0x3C,0xC6,0x39,0xF8,0x00,0x00,
  0x00,0x00,0x07,0xFF,0xE3,0x07,0x7F,0xCF,0xE3,0xF7,0x38,0x06,0x1F,0xB8,0x00,0x00,
  0x00,0x00,0x3F,0xC0,0x46,0x06,0x7D,0x83,0xC3,0xE3,0x30,0x06,0x1F,0x18,0x00,0x00,
  0x00,0x00,0x07,0x80,0xC6,0x06,0x78,0x83,0x81,0x83,0x00,0x06,0x0C,0x18,0x00,0x00,
  0x00,0x00,0x07,0x00,0x00,0x02,0x20,0x07,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x00,
  0x00,0x00,0x0E,0x00,0x00,0x00,0x00,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x04,0x00,0x00,0x00,0x00,0x0E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

/**
 * Made with Marlin Bitmap Converter
 * https://marlinfw.org/tools/u8glib/converter.html
 *
 * This bitmap from the file 'berlin1.png'
 */
#define berlin_BMPWIDTH  63

const unsigned char bitmap_berlin1[] = {
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,
  0xFC,0xE0,0x03,0x9F,0xFF,0xFF,0xFF,0xFE,
  0xFC,0xE0,0x03,0x9F,0xFF,0xFF,0xFF,0xFE,
  0xFC,0x00,0x00,0x1F,0xFF,0xFF,0xFF,0xFE,
  0xFC,0x00,0x00,0x1F,0xFF,0xFF,0xFF,0xFE,
  0xFC,0x00,0x00,0x1F,0xFF,0xFF,0xFF,0xFE,
  0xFC,0x00,0x00,0x03,0xFF,0xFF,0xFF,0xFE,
  0xFC,0x00,0x00,0x03,0xFF,0xFF,0xFF,0xFE,
  0xE0,0xE0,0xE0,0x00,0x7F,0xFF,0xFF,0xFE,
  0xE0,0xE0,0xE0,0x00,0x7F,0xFF,0xFF,0xFE,
  0xE0,0xE0,0xE0,0x00,0x7F,0xFF,0xFF,0xFE,
  0xE0,0x00,0x00,0x00,0x0F,0xFF,0xFF,0xFE,
  0xE0,0x00,0x00,0x00,0x0F,0xFF,0xFF,0xFE,
  0xE0,0x00,0x00,0x00,0x0F,0xFF,0xFF,0xFE,
  0xE0,0x1F,0x00,0x00,0x00,0x7F,0xFF,0xFE,
  0xE0,0x1F,0x00,0x00,0x00,0x7F,0xFF,0xFE,
  0xE3,0x1C,0x1C,0x00,0x00,0x00,0x00,0x0E,
  0xE3,0x1C,0x1C,0x00,0x00,0x00,0x00,0x0E,
  0xE3,0x1C,0x1C,0x00,0x00,0x00,0x00,0x0E,
  0xE0,0xFF,0xE0,0x00,0x00,0x00,0x0F,0xFE,
  0xE0,0xFF,0xE0,0x00,0x00,0x00,0x0F,0xFE,
  0xE0,0xFF,0xE0,0x00,0x00,0x00,0x0F,0xFE,
  0xE0,0x00,0x00,0x00,0x00,0x00,0x0F,0xFE,
  0xE0,0x00,0x00,0x00,0x00,0x00,0x0F,0xFE,
  0xE0,0x00,0x00,0x00,0x00,0x00,0x0F,0xFE,
  0xE0,0x00,0x00,0x00,0x00,0x00,0x0F,0xFE,
  0xE0,0x00,0x00,0x00,0x00,0x00,0x0F,0xFE,
  0xE0,0x00,0x00,0x00,0x00,0x00,0x0F,0xFE,
  0xE0,0x00,0x00,0x00,0x00,0x00,0x0F,0xFE,
  0xE0,0x00,0x00,0x00,0x00,0x00,0x0F,0xFE,
  0xE0,0x00,0x00,0x00,0x00,0x00,0x0F,0xFE,
  0xE0,0x00,0x00,0x00,0x00,0x00,0x0F,0xFE,
  0xE0,0x00,0x00,0x00,0x00,0x00,0x7F,0xFE,
  0xE0,0x00,0x00,0x00,0x00,0x00,0x7F,0xFE,
  0xE0,0x00,0x00,0x00,0x00,0x00,0x7F,0xFE,
  0xFC,0x00,0x00,0x00,0x00,0x00,0x7F,0xFE,
  0xFC,0x00,0x00,0x00,0x00,0x00,0x7F,0xFE,
  0xFC,0x1F,0x03,0xFF,0x83,0xF0,0x7F,0xFE,
  0xFC,0x1F,0x03,0xFF,0x83,0xF0,0x7F,0xFE,
  0xFC,0x1F,0x03,0xFF,0x83,0xF0,0x7F,0xFE,
  0xFC,0x1F,0x1F,0xFF,0x8F,0xF0,0x7F,0xFE,
  0xFC,0x1F,0x1F,0xFF,0x8F,0xF0,0x7F,0xFE,
  0xFC,0x1F,0x1F,0xFF,0x8F,0xF0,0x7F,0xFE,
  0xFC,0xFF,0xFF,0xFF,0xFF,0xF1,0xFF,0xFE,
  0xFC,0xFF,0xFF,0xFF,0xFF,0xF1,0xFF,0xFE,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE
};

/**
 * Made with Marlin Bitmap Converter
 * https://marlinfw.org/tools/u8glib/converter.html
 *
 * This bitmap from the file 'berlin2.png'
 */

const unsigned char bitmap_berlin2[] = {
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,
  0xFC,0xE0,0x03,0x9F,0xFF,0xFF,0xFF,0xFE,
  0xFC,0xE0,0x03,0x9F,0xFF,0xFF,0xFF,0xFE,
  0xFC,0x00,0x00,0x1F,0xFF,0xFF,0xFF,0xFE,
  0xFC,0x00,0x00,0x1F,0xFF,0xFF,0xFF,0xFE,
  0xFC,0x00,0x00,0x1F,0xFF,0xFF,0xFF,0xFE,
  0xFC,0x00,0x00,0x03,0xFF,0xFF,0xFF,0xFE,
  0xFC,0x00,0x00,0x03,0xFF,0xFF,0xFF,0xFE,
  0xE0,0xE0,0xE0,0x00,0x7F,0xFF,0x8F,0xFE,
  0xE0,0xE0,0xE0,0x00,0x7F,0xFF,0x8F,0xFE,
  0xE0,0xE0,0xE0,0x00,0x7F,0xFF,0x8F,0xFE,
  0xE0,0x00,0x00,0x00,0x0F,0xFF,0x8F,0xFE,
  0xE0,0x00,0x00,0x00,0x0F,0xFF,0x8F,0xFE,
  0xE0,0x00,0x00,0x00,0x0F,0xFF,0x8F,0xFE,
  0xE0,0x1F,0x00,0x00,0x00,0x7F,0x8F,0xFE,
  0xE0,0x1F,0x00,0x00,0x00,0x7F,0x8F,0xFE,
  0xE3,0x1C,0x1C,0x00,0x00,0x00,0x0F,0xFE,
  0xE3,0x1C,0x1C,0x00,0x00,0x00,0x0F,0xFE,
  0xE3,0x1C,0x1C,0x00,0x00,0x00,0x0F,0xFE,
  0xE0,0xFF,0xE0,0x00,0x00,0x00,0x0F,0xFE,
  0xE0,0xFF,0xE0,0x00,0x00,0x00,0x0F,0xFE,
  0xE0,0xFF,0xE0,0x00,0x00,0x00,0x0F,0xFE,
  0xE0,0x00,0x00,0x00,0x00,0x00,0x0F,0xFE,
  0xE0,0x00,0x00,0x00,0x00,0x00,0x0F,0xFE,
  0xE0,0x00,0x00,0x00,0x00,0x00,0x0F,0xFE,
  0xE0,0x00,0x00,0x00,0x00,0x00,0x0F,0xFE,
  0xE0,0x00,0x00,0x00,0x00,0x00,0x0F,0xFE,
  0xE0,0x00,0x00,0x00,0x00,0x00,0x0F,0xFE,
  0xE0,0x00,0x00,0x00,0x00,0x00,0x0F,0xFE,
  0xE0,0x00,0x00,0x00,0x00,0x00,0x0F,0xFE,
  0xE0,0x00,0x00,0x00,0x00,0x00,0x0F,0xFE,
  0xE0,0x00,0x00,0x00,0x00,0x00,0x0F,0xFE,
  0xE0,0x00,0x00,0x00,0x00,0x00,0x7F,0xFE,
  0xE0,0x00,0x00,0x00,0x00,0x00,0x7F,0xFE,
  0xE0,0x00,0x00,0x00,0x00,0x00,0x7F,0xFE,
  0xFC,0x00,0x00,0x00,0x00,0x00,0x7F,0xFE,
  0xFC,0x00,0x00,0x00,0x00,0x00,0x7F,0xFE,
  0xFC,0x1F,0x03,0xFF,0x83,0xF0,0x7F,0xFE,
  0xFC,0x1F,0x03,0xFF,0x83,0xF0,0x7F,0xFE,
  0xFC,0x1F,0x03,0xFF,0x83,0xF0,0x7F,0xFE,
  0xFC,0x7F,0x03,0xFF,0x83,0xF1,0xFF,0xFE,
  0xFC,0x7F,0x03,0xFF,0x83,0xF1,0xFF,0xFE,
  0xFC,0x7F,0x03,0xFF,0x83,0xF1,0xFF,0xFE,
  0xFF,0xFF,0x03,0xFF,0x8F,0xFF,0xFF,0xFE,
  0xFF,0xFF,0x1F,0xFF,0x8F,0xFF,0xFF,0xFE,
  0xFF,0xFF,0x1F,0xFF,0xFF,0xFF,0xFF,0xFE,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,
  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE
};


void berlinDance(void)
{
	u8g2_ClearDisplay(&u8g2); 	//Clear display
	
	for(int i=0;i<10;i++)
	{
		
		u8g2_ClearBuffer(&u8g2);	//Clear buffer to send information
		u8g2_DrawBitmap(&u8g2, 30, 16, 8, 50, bitmap_berlin1); // Draw the logo into the buffer
		u8g2_SendBuffer(&u8g2);		//Send the bufferinto the screen.

		vTaskDelay(pdMS_TO_TICKS(150));

		u8g2_ClearBuffer(&u8g2);	//Clear buffer to send information
		u8g2_DrawBitmap(&u8g2, 30, 16, 8, 50, bitmap_berlin2); // Draw the logo into the buffer
		u8g2_SendBuffer(&u8g2);		//Send the bufferinto the screen.

		vTaskDelay(pdMS_TO_TICKS(150));
	}
}

void splashScreen(void)
{
	// berlinDance();

	u8g2_ClearDisplay(&u8g2); 	//Clear display
	u8g2_ClearBuffer(&u8g2);	//Clear buffer to send information
	u8g2_DrawBitmap(&u8g2, 0, 16, 16, 32, bitmap_deepdeck); // Draw the logo into the buffer
	u8g2_SendBuffer(&u8g2);		//Send the bufferinto the screen.

	vTaskDelay(pdMS_TO_TICKS(1000));

	u8g2_ClearDisplay(&u8g2); 	//Clear display
	u8g2_ClearBuffer(&u8g2);	//Clear buffer to send information
	u8g2_DrawBitmap(&u8g2, 0, 16, 16, 32, bitmap_ahuyama); // Draw the logo into the buffer
	u8g2_SendBuffer(&u8g2);		//Send the bufferinto the screen.
}
