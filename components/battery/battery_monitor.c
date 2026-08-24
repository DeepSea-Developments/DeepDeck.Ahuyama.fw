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
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "keyboard_config.h"

#include "battery_monitor.h"

#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   500          //Multisampling

static const char *TAG = "BATT";

static adc_oneshot_unit_handle_t adc_handle = NULL;
static adc_cali_handle_t adc_cali_handle = NULL;
static const adc_channel_t channel = BATT_PIN;
static const adc_atten_t atten = ADC_ATTEN_DB_2_5;

uint32_t voltage = 0;

//static esp_adc_cal_characteristics_t *adc_chars;
//check battery level
uint32_t get_battery_level(void) {
	int64_t adc_sum = 0;
	int adc_reading = 0;
	int raw = 0;
	int voltage_mv = 0;

	if (adc_handle == NULL) {
		ESP_LOGE(TAG, "ADC not initialised");
		return 0;
	}

	// Multisampling
	for (int i = 0; i < NO_OF_SAMPLES; i++) {
		if (adc_oneshot_read(adc_handle, channel, &raw) != ESP_OK) {
			ESP_LOGE(TAG, "ADC read failed");
			return 0;
		}
		adc_sum += raw;
	}
	adc_reading = (int)(adc_sum / NO_OF_SAMPLES);

	// Prefer the per-chip eFuse calibration. The fallback is only a rough
	// approximation: ADC_ATTEN_DB_2_5 puts full scale near 1250mV, not the
	// 1100mV internal reference.
	if (adc_cali_handle == NULL ||
	    adc_cali_raw_to_voltage(adc_cali_handle, adc_reading, &voltage_mv) != ESP_OK) {
		voltage_mv = (adc_reading * 1250) / 4095;
	}

	voltage = (uint32_t)voltage_mv;

	// Clamp before subtracting: these are unsigned, and a battery at or below
	// Vout_min would otherwise wrap the percentage to ~4.29 billion.
	if (voltage_mv <= (int)Vout_min) {
		return 0;
	}
	if (voltage_mv >= (int)Vout_max) {
		return 100;
	}

	return (uint32_t)((voltage_mv - Vout_min) * 100 / (Vout_max - Vout_min));
}

//initialize battery monitor pin
void init_batt_monitor(void) {
	adc_oneshot_unit_init_cfg_t init_config = {
		.unit_id = ADC_UNIT_1,
		.ulp_mode = ADC_ULP_MODE_DISABLE,
	};
	esp_err_t err = adc_oneshot_new_unit(&init_config, &adc_handle);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "adc_oneshot_new_unit failed: %s", esp_err_to_name(err));
		adc_handle = NULL;
		return;
	}

	adc_oneshot_chan_cfg_t config = {
		.atten = atten,
		.bitwidth = ADC_BITWIDTH_12,
	};
	err = adc_oneshot_config_channel(adc_handle, channel, &config);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "adc_oneshot_config_channel failed: %s", esp_err_to_name(err));
		adc_oneshot_del_unit(adc_handle);
		adc_handle = NULL;
		return;
	}

	// Calibration is optional - without eFuse data the fallback above is used.
	adc_cali_line_fitting_config_t cali_config = {
		.unit_id = ADC_UNIT_1,
		.atten = atten,
		.bitwidth = ADC_BITWIDTH_12,
		/* Only consulted on chips with neither Two Point nor Vref burnt into
		 * eFuse; the scheme refuses to be created if it is left at zero. */
		.default_vref = DEFAULT_VREF,
	};
	if (adc_cali_create_scheme_line_fitting(&cali_config, &adc_cali_handle) != ESP_OK) {
		ESP_LOGW(TAG, "no ADC calibration available, using approximation");
		adc_cali_handle = NULL;
	}
}
