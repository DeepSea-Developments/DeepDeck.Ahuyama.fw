/*
 * This battery monitoring code is based on reading the voltage
 * after after a voltage divider and checking the level on an analog pin
 * Based on the adc example from Espressif
 *
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
#include <time.h>
#include <string.h>
#include <assert.h>

#include "esp_system.h"
#include "esp_adc/adc_oneshot.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "keyboard_config.h"

#include "battery_monitor.h"

#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   500          //Multisampling

static adc_oneshot_unit_handle_t adc_handle;
static const adc_channel_t channel = BATT_PIN;
static const adc_atten_t atten = ADC_ATTEN_DB_2_5;

uint32_t voltage = 0;

//static esp_adc_cal_characteristics_t *adc_chars;
//check battery level
uint32_t get_battery_level(void) {
    int adc_reading = 0;
    int raw = 0;
	//Multisampling

	for (int i = 0; i < NO_OF_SAMPLES; i++) {
        adc_oneshot_read(adc_handle, channel, &raw);
        adc_reading += raw;
	}
	adc_reading /= NO_OF_SAMPLES;

	//Convert adc_reading to voltage in mV
	// For ESP-IDF v5.x, use adc_cali_raw_to_voltage or calculate manually
    // Here, we use the formula: voltage = (adc_reading * reference_voltage) / max_adc
    // For 12-bit ADC, max_adc = 4095, reference_voltage = 1100mV (default)
    voltage = (adc_reading * 1100) / 4095;

    uint32_t battery_percent = ((voltage - Vout_min) * 100 / (Vout_max - Vout_min));
    // printf("Raw: %d\tVoltage: %dmV\tPercent: %d\n", adc_reading, voltage, battery_percent);
    return battery_percent;

}

//initialize battery monitor pin
void init_batt_monitor(void) {
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    adc_oneshot_new_unit(&init_config, &adc_handle);

    adc_oneshot_chan_cfg_t config = {
        .atten = atten,
        .bitwidth = ADC_BITWIDTH_12,
    };
    adc_oneshot_config_channel(adc_handle, channel, &config);
}
