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

#include <stdio.h>
#include "driver/i2c.h"
#include "apds9960.h"
#include "esp_log.h"
#include <string.h>
#include <string.h>

#define APDS9960_TIMEOUT_MS_DEFAULT (1000)

// #define DEBUG_APDS9960 /*uncomment to enable debug*/
typedef struct
{
	i2c_bus_device_handle_t i2c_dev;
	uint8_t dev_addr;
	uint32_t timeout;
	apds9960_control_t _control_t; /*< config control register>*/
	apds9960_enable_t _enable_t;   /*< config enable register>*/
	apds9960_config1_t _config1_t; /*< config config1 register>*/
	apds9960_config2_t _config2_t; /*< config config2 register>*/
	apds9960_config3_t _config3_t; /*< config config3 register>*/
	apds9960_gconf1_t _gconf1_t;   /*< config gconfig1 register>*/
	apds9960_gconf2_t _gconf2_t;   /*< config gconfig2 register>*/
	apds9960_gconf3_t _gconf3_t;   /*< config gconfig3 register>*/
	apds9960_gconf4_t _gconf4_t;   /*< config gconfig4 register>*/
	apds9960_status_t _status_t;   /*< config status register>*/
	apds9960_gstatus_t _gstatus_t; /*< config gstatus register>*/
	apds9960_propulse_t _ppulse_t; /*< config pro pulse register>*/
	apds9960_gespulse_t _gpulse_t; /*< config ges pulse register>*/
	apds9960_pers_t _pers_t;	   /*< config pers register>*/
	uint8_t gest_cnt;			   /*< counter of gesture >*/
	uint8_t up_cnt;				   /*< counter of up gesture >*/
	uint8_t down_cnt;			   /*< counter of down gesture >*/
	uint8_t left_cnt;			   /*< counter of left gesture >*/
	uint8_t right_cnt;			   /*< counter of right gesture >*/
} apds9960_dev_t;

static float __powf(const float x, const float y)
{
	return (float)(pow((double)x, (double)y));
}

apds9960_handle_t apds9960_create(i2c_bus_handle_t bus, uint8_t dev_addr)
{

	apds9960_dev_t *sens = (apds9960_dev_t *)calloc(1, sizeof(apds9960_dev_t));
	sens->i2c_dev = i2c_bus_device_create(bus, dev_addr,
										  i2c_bus_get_current_clk_speed(bus));
	if (sens->i2c_dev == NULL)
	{
		free(sens);

		return NULL;
	}
	sens->dev_addr = dev_addr;
	sens->timeout = APDS9960_TIMEOUT_MS_DEFAULT;

	return (apds9960_handle_t)sens;
}

esp_err_t apds9960_delete(apds9960_handle_t *sensor)
{

	if (*sensor == NULL)
	{

		return ESP_OK;
	}

	apds9960_dev_t *sens = (apds9960_dev_t *)(*sensor);
	i2c_bus_device_delete(&sens->i2c_dev);
	free(sens);
	*sensor = NULL;

	return ESP_OK;
}


esp_err_t apds9960_gesture_init(apds9960_handle_t sensor)
{

	apds9960_enable_gesture_engine(sensor, false);
	apds9960_enable_proximity_engine(sensor, false);
	apds9960_enable_color_engine(sensor, false);
	apds9960_enable_color_interrupt(sensor, false);
	apds9960_enable_proximity_interrupt(sensor, false);
	apds9960_enable(sensor, false);
	apds9960_enable(sensor, true);
	/* Set default values for ambient light and proximity registers */
	apds9960_set_adc_integration_time(sensor, DEFAULT_ATIME);
	apds9960_set_register(sensor, APDS9960_WTIME, DEFAULT_WTIME);

	apds9960_set_register(sensor, APDS9960_PPULSE, DEFAULT_PROX_PPULSE);
	apds9960_set_register(sensor, APDS9960_POFFSET_UR, DEFAULT_POFFSET_UR);
	apds9960_set_register(sensor, APDS9960_POFFSET_DL, DEFAULT_POFFSET_DL);
	apds9960_set_register(sensor, APDS9960_CONFIG1, DEFAULT_CONFIG1);

	apds9960_set_led_drive_boost(sensor, APDS9960_LEDDRIVE_100MA,
								 APDS9960_LEDBOOST_100PCNT);
	apds9960_set_proximity_gain(sensor, DEFAULT_PGAIN);
	apds9960_set_ambient_light_gain(sensor, DEFAULT_AGAIN);

	apds9960_set_proximity_interrupt_threshold(sensor, DEFAULT_PILT,
											   DEFAULT_PIHT, DEFAULT_PERS);

	apds9960_set_register(sensor, APDS9960_PERS, DEFAULT_PERS);
	apds9960_set_register(sensor, APDS9960_CONFIG2, DEFAULT_CONFIG2);
	apds9960_set_register(sensor, APDS9960_CONFIG3, DEFAULT_CONFIG3);

	apds9960_set_light_inthigh_threshold(sensor, DEFAULT_AIHT);
	apds9960_set_light_intlow_threshold(sensor, DEFAULT_AILT);

	//	apds9960_enable_gesture_interrupt(sensor, true);
	//	apds9960_clear_interrupt(sensor);
	/////////
	apds9960_set_gesture_enter_thresh(sensor, DEFAULT_GPENTH);
	apds9960_set_gesture_exit_thresh(sensor, DEFAULT_GEXTH);
	////////

	apds9960_set_gesture_dimensions(sensor, APDS9960_DIMENSIONS_ALL);
	apds9960_set_gesture_fifo_threshold(sensor, APDS9960_GFIFO_4);
	apds9960_set_gesture_gain(sensor, APDS9960_GGAIN_4X);

	apds9960_reset_counts(sensor);

	apds9960_set_gesture_waittime(sensor, APDS9960_GWTIME_2_8MS);
	apds9960_set_gesture_pulse(sensor, APDS9960_GPULSELEN_32US, 8);

	apds9960_enable_proximity_engine(sensor, true);
	apds9960_enable_proximity_interrupt(sensor, false);

	apds9960_enable_gesture_interrupt(sensor, true);
	apds9960_set_gesture_Int_Enable(sensor, 1);
	apds9960_enable_gesture_engine(sensor, true);

	//	return apds9960_enable_gesture_engine(sensor, true);

	return ESP_OK;
}

esp_err_t apds9960_set_register(apds9960_handle_t sensor, uint8_t apds_reg,
								uint16_t reg_val)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	return i2c_bus_write_byte(sens->i2c_dev, apds_reg, reg_val);
}

uint8_t apds9960_get_enable(apds9960_handle_t sensor)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	return (sens->_enable_t.gen << 6) | (sens->_enable_t.pien << 5) | (sens->_enable_t.aien << 4) | (sens->_enable_t.wen << 3) | (sens->_enable_t.pen << 2) | (sens->_enable_t.aen << 1) | sens->_enable_t.pon;
}

uint8_t apds9960_get_pers(apds9960_handle_t sensor)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	return (sens->_pers_t.ppers << 4) | sens->_pers_t.apers;
}

uint8_t apds9960_get_ppulse(apds9960_handle_t sensor)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	return (sens->_ppulse_t.pplen << 6) | sens->_ppulse_t.ppulse;
}

uint8_t apds9960_get_gpulse(apds9960_handle_t sensor)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	return (sens->_gpulse_t.gplen << 6) | sens->_gpulse_t.gpulse;
}

uint8_t apds9960_get_control(apds9960_handle_t sensor)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	return (sens->_control_t.leddrive << 6) | (sens->_control_t.pgain << 2) | sens->_control_t.again;
}

uint8_t apds9960_get_config1(apds9960_handle_t sensor)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	return sens->_config1_t.wlong << 1;
}

uint8_t apds9960_get_config2(apds9960_handle_t sensor)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	return (sens->_config2_t.psien << 7) | (sens->_config2_t.cpsien << 6) | (sens->_config2_t.led_boost << 4) | 1;
}

uint8_t apds9960_get_config3(apds9960_handle_t sensor)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	return (sens->_config3_t.pcmp << 5) | (sens->_config3_t.sai << 4) | (sens->_config3_t.pmask_u << 3) | (sens->_config3_t.pmask_d << 2) | (sens->_config3_t.pmask_l << 1) | sens->_config3_t.pmask_r;
}

void apds9960_set_status(apds9960_handle_t sensor, uint8_t data)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	sens->_status_t.avalid = data & 0x01;
	sens->_status_t.pvalid = (data >> 1) & 0x01;
	sens->_status_t.gint = (data >> 2) & 0x01;
	sens->_status_t.aint = (data >> 4) & 0x01;
	sens->_status_t.pint = (data >> 5) & 0x01;
	sens->_status_t.pgsat = (data >> 6) & 0x01;
	sens->_status_t.cpsat = (data >> 7) & 0x01;
}

void apds9960_set_gstatus(apds9960_handle_t sensor, uint8_t data)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	sens->_gstatus_t.gfov = (data >> 1) & 0x01;
	sens->_gstatus_t.gvalid = data & 0x01;
}

uint8_t apds9960_get_gconf1(apds9960_handle_t sensor)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	return (sens->_gconf1_t.gfifoth << 7) | (sens->_gconf1_t.gexmsk << 5) | sens->_gconf1_t.gexpers;
}

uint8_t apds9960_get_gconf2(apds9960_handle_t sensor)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	return (sens->_gconf2_t.ggain << 5) | (sens->_gconf2_t.gldrive << 3) | sens->_gconf2_t.gwtime;
}

uint8_t apds9960_get_gconf3(apds9960_handle_t sensor)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	return sens->_gconf3_t.gdims;
}

uint8_t apds9960_get_gconf4(apds9960_handle_t sensor)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	return (sens->_gconf4_t.gien << 1) | sens->_gconf4_t.gmode;
}

void apds9960_set_gconf4(apds9960_handle_t sensor, uint8_t data)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	sens->_gconf4_t.gien = (data >> 1) & 0x01;
	sens->_gconf4_t.gmode = data & 0x01;
}

void apds9960_reset_counts(apds9960_handle_t sensor)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	sens->gest_cnt = 0;
	sens->up_cnt = 0;
	sens->down_cnt = 0;
	sens->left_cnt = 0;
	sens->right_cnt = 0;
}

esp_err_t apds9960_set_timeout(apds9960_handle_t sensor, uint32_t tout_ms)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	sens->timeout = tout_ms;
	return ESP_OK;
}

esp_err_t apds9960_get_deviceid(apds9960_handle_t sensor, uint8_t *deviceid)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	esp_err_t ret;
	uint8_t tmp;
	ret = i2c_bus_read_byte(sens->i2c_dev, APDS9960_WHO_AM_I_REG, &tmp);
	*deviceid = tmp;
	return ret;
}

esp_err_t apds9960_set_mode(apds9960_handle_t sensor, apds9960_mode_t mode)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	uint8_t tmp;

	if (i2c_bus_read_byte(sens->i2c_dev, APDS9960_MODE_ENABLE, &tmp) != ESP_OK)
	{
		return ESP_FAIL;
	}

	tmp |= mode;
	return i2c_bus_write_byte(sens->i2c_dev, APDS9960_MODE_ENABLE, tmp);
}

apds9960_mode_t apds9960_get_mode(apds9960_handle_t sensor)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	uint8_t value;

	/* Read current ENABLE register */
	if (i2c_bus_read_byte(sens->i2c_dev, APDS9960_MODE_ENABLE, &value) != ESP_OK)
	{
		return ESP_FAIL;
	}

	return (apds9960_mode_t)value;
}

esp_err_t apds9960_enable_gesture_engine(apds9960_handle_t sensor, bool en)
{
	esp_err_t ret;
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;

	if (!en)
	{
		sens->_gconf4_t.gmode = 0;
		if (i2c_bus_write_byte(sens->i2c_dev, APDS9960_GCONF4,
							   (sens->_gconf4_t.gien << 1) | sens->_gconf4_t.gmode) != ESP_OK)
		{
			return ESP_FAIL;
		}
	}

	sens->_enable_t.gen = en;
	ret = i2c_bus_write_byte(sens->i2c_dev, APDS9960_MODE_ENABLE,
							 (sens->_enable_t.gen << 6) | (sens->_enable_t.pien << 5) | (sens->_enable_t.aien << 4) | (sens->_enable_t.wen << 3) | (sens->_enable_t.pen << 2) | (sens->_enable_t.aen << 1) | sens->_enable_t.pon);
	apds9960_reset_counts(sensor);
	return ret;
}

esp_err_t apds9960_set_led_drive_boost(apds9960_handle_t sensor,
									   apds9960_leddrive_t drive, apds9960_ledboost_t boost)
{
	// set BOOST
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	sens->_config2_t.led_boost = boost;

	if (i2c_bus_write_byte(sens->i2c_dev, APDS9960_CONFIG2,
						   (sens->_config2_t.psien << 7) | (sens->_config2_t.cpsien << 6) | (sens->_config2_t.led_boost << 4) | 1) != ESP_OK)
	{
		return ESP_FAIL;
	}

	sens->_control_t.leddrive = drive;
	return i2c_bus_write_byte(sens->i2c_dev, APDS9960_CONTROL,
							  ((sens->_control_t.leddrive << 6) | (sens->_control_t.pgain << 2) | sens->_control_t.again));
}

esp_err_t apds9960_set_wait_time(apds9960_handle_t sensor, uint8_t time)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	return i2c_bus_write_byte(sens->i2c_dev, APDS9960_WTIME, time);
}

esp_err_t apds9960_set_adc_integration_time(apds9960_handle_t sensor,
											uint16_t iTimeMS)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	float temp;
	// convert ms into 2.78ms increments
	temp = iTimeMS;
	temp /= 2.78;
	temp = 256 - temp;

	if (temp > 255)
	{
		temp = 255;
	}
	else if (temp < 0)
	{
		temp = 0;
	}

	/* Update the timing register */
	return i2c_bus_write_byte(sens->i2c_dev, APDS9960_ATIME, (uint8_t)temp);
}

float apds9960_get_adc_integration_time(apds9960_handle_t sensor)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	float temp;
	uint8_t val;

	if (i2c_bus_read_byte(sens->i2c_dev, APDS9960_ATIME, &val) != ESP_OK)
	{
		return ESP_FAIL;
	}

	temp = val;
	// convert to units of 2.78 ms
	temp = 256 - temp;
	temp *= 2.78;
	return temp;
}

esp_err_t apds9960_set_ambient_light_gain(apds9960_handle_t sensor,
										  apds9960_again_t again)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	sens->_control_t.again = again;

	/* Update the timing register */
	return i2c_bus_write_byte(sens->i2c_dev, APDS9960_CONTROL,
							  ((sens->_control_t.leddrive << 6) | (sens->_control_t.pgain << 2) | sens->_control_t.again));
}

apds9960_again_t apds9960_get_ambient_light_gain(apds9960_handle_t sensor)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	uint8_t val;

	if (i2c_bus_read_byte(sens->i2c_dev, APDS9960_CONTROL, &val) != ESP_OK)
	{
		return ESP_FAIL;
	}

	return (apds9960_again_t)(val & 0x03);
}

esp_err_t apds9960_set_gesture_Int_Enable(apds9960_handle_t sensor,
										  uint8_t enable)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	uint8_t val;

	/* Read value from GCONF4 register */
	if (i2c_bus_read_byte(sens->i2c_dev, APDS9960_GCONF4, &val) != ESP_OK)
	{
		return ESP_FAIL;
	}

	//    if( !wireReadDataByte(APDS9960_GCONF4, val) ) {
	//        return false;
	//    }

	/* Set bits in register to given value */
	enable &= 0b00000001;
	enable = enable << 1;
	val &= 0b11111101;
	val |= enable;

	/* Write register value back into GCONF4 register */
	//    if( !wireWriteDataByte(APDS9960_GCONF4, val) ) {
	//        return false;
	//    }
	return i2c_bus_write_byte(sens->i2c_dev, APDS9960_GCONF4, val);
}

esp_err_t apds9960_enable_gesture_interrupt(apds9960_handle_t sensor, bool en)
{
	esp_err_t ret;
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	sens->_enable_t.gen = en;
	ret = i2c_bus_write_byte(sens->i2c_dev, APDS9960_MODE_ENABLE,
							 ((sens->_enable_t.gen << 6) | (sens->_enable_t.pien << 5) | (sens->_enable_t.aien << 4) | (sens->_enable_t.wen << 3) | (sens->_enable_t.pen << 2) | (sens->_enable_t.aen << 1) | sens->_enable_t.pon));
	apds9960_clear_interrupt(sensor);
	return ret;
}

esp_err_t apds9960_enable_proximity_engine(apds9960_handle_t sensor, bool en)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	sens->_enable_t.pen = en;
	return i2c_bus_write_byte(sens->i2c_dev, APDS9960_MODE_ENABLE,
							  ((sens->_enable_t.gen << 6) | (sens->_enable_t.pien << 5) | (sens->_enable_t.aien << 4) | (sens->_enable_t.wen << 3) | (sens->_enable_t.pen << 2) | (sens->_enable_t.aen << 1) | sens->_enable_t.pon));
}

esp_err_t apds9960_set_proximity_gain(apds9960_handle_t sensor,
									  apds9960_pgain_t pgain)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	sens->_control_t.pgain = pgain;

	/* Update the timing register */
	return i2c_bus_write_byte(sens->i2c_dev, APDS9960_CONTROL,
							  (sens->_control_t.leddrive << 6) | (sens->_control_t.pgain << 2) | sens->_control_t.again);
}

apds9960_pgain_t apds9960_get_proximity_gain(apds9960_handle_t sensor)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	uint8_t val;

	if (i2c_bus_read_byte(sens->i2c_dev, APDS9960_CONTROL, &val) != ESP_OK)
	{
		return ESP_FAIL;
	}

	return (apds9960_pgain_t)(val & 0x0C);
}

esp_err_t apds9960_set_proximity_pulse(apds9960_handle_t sensor,
									   apds9960_ppulse_len_t pLen, uint8_t pulses)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;

	if (pulses < 1)
	{
		pulses = 1;
	}
	else if (pulses > 64)
	{
		pulses = 64;
	}

	pulses--;
	sens->_ppulse_t.pplen = pLen;
	sens->_ppulse_t.ppulse = pulses;
	return i2c_bus_write_byte(sens->i2c_dev, APDS9960_PPULSE,
							  (sens->_ppulse_t.pplen << 6) | sens->_ppulse_t.ppulse);
}

esp_err_t apds9960_enable_color_engine(apds9960_handle_t sensor, bool en)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	sens->_enable_t.aen = en;
	return i2c_bus_write_byte(sens->i2c_dev, APDS9960_MODE_ENABLE,
							  ((sens->_enable_t.gen << 6) | (sens->_enable_t.pien << 5) | (sens->_enable_t.aien << 4) | (sens->_enable_t.wen << 3) | (sens->_enable_t.pen << 2) | (sens->_enable_t.aen << 1) | sens->_enable_t.pon));
}

bool apds9960_color_data_ready(apds9960_handle_t sensor)
{
	uint8_t data;
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	i2c_bus_read_byte(sens->i2c_dev, APDS9960_STATUS, &data);
	apds9960_set_status(sensor, data);
	return sens->_status_t.avalid;
}

esp_err_t apds9960_get_color_data(apds9960_handle_t sensor, uint16_t *r,
								  uint16_t *g, uint16_t *b, uint16_t *c)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	uint8_t data[2] = {0};
	i2c_bus_read_bytes(sens->i2c_dev, APDS9960_CDATAL, 2, data);
	*c = (data[0] << 8) | data[1];
	i2c_bus_read_bytes(sens->i2c_dev, APDS9960_RDATAL, 2, data);
	*r = (data[0] << 8) | data[1];
	i2c_bus_read_bytes(sens->i2c_dev, APDS9960_GDATAL, 2, data);
	*g = (data[0] << 8) | data[1];
	i2c_bus_read_bytes(sens->i2c_dev, APDS9960_BDATAL, 2, data);
	*b = (data[0] << 8) | data[1];
	return ESP_OK;
}

uint16_t apds9960_calculate_color_temperature(apds9960_handle_t sensor,
											  uint16_t r, uint16_t g, uint16_t b)
{
	float rgb_xcorrelation, rgb_ycorrelation, rgb_zcorrelation; /* RGB to XYZ correlation      */
	float chromaticity_xc, chromaticity_yc;						/* Chromaticity co-ordinates   */
	float formula;												/* McCamy's formula            */
	float cct;

	/* 1. Map RGB values to their XYZ counterparts.    */
	/* Based on 6500K fluorescent, 3000K fluorescent   */
	/* and 60W incandescent values for a wide range.   */
	/* Note: Y = Illuminance or lux                    */
	rgb_xcorrelation = (-0.14282F * r) + (1.54924F * g) + (-0.95641F * b);
	rgb_ycorrelation = (-0.32466F * r) + (1.57837F * g) + (-0.73191F * b);
	rgb_zcorrelation = (-0.68202F * r) + (0.77073F * g) + (0.56332F * b);

	/* 2. Calculate the chromaticity co-ordinates      */
	chromaticity_xc = (rgb_xcorrelation) / (rgb_xcorrelation);
	chromaticity_yc = (rgb_ycorrelation) / (rgb_xcorrelation + rgb_ycorrelation + rgb_zcorrelation);

	/* 3. Use McCamy's formula to determine the CCT    */
	formula = (chromaticity_xc - 0.3320F) / (0.1858F - chromaticity_yc);

	/* Calculate the final CCT */
	cct = (449.0F * __powf(formula, 3)) + (3525.0F * __powf(formula, 2)) + (6823.3F * formula) + 5520.33F;

	/* Return the results in degrees Kelvin */
	return (uint16_t)cct;
}

uint16_t apds9960_calculate_lux(apds9960_handle_t sensor, uint16_t r,
								uint16_t g, uint16_t b)
{
	float illuminance;

	/* This only uses RGB ... how can we integrate clear or calculate lux */
	/* based exclusively on clear since this might be more reliable?      */
	illuminance = (-0.32466F * r) + (1.57837F * g) + (-0.73191F * b);

	return (uint16_t)illuminance;
}

esp_err_t apds9960_enable_color_interrupt(apds9960_handle_t sensor, bool en)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	sens->_enable_t.aien = en;
	return i2c_bus_write_byte(sens->i2c_dev, APDS9960_MODE_ENABLE,
							  ((sens->_enable_t.gen << 6) | (sens->_enable_t.pien << 5) | (sens->_enable_t.aien << 4) | (sens->_enable_t.wen << 3) | (sens->_enable_t.pen << 2) | (sens->_enable_t.aen << 1) | sens->_enable_t.pon));
}

esp_err_t apds9960_set_int_limits(apds9960_handle_t sensor, uint16_t low,
								  uint16_t high)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	i2c_bus_write_byte(sens->i2c_dev, APDS9960_AILTL, low & 0xFF);
	i2c_bus_write_byte(sens->i2c_dev, APDS9960_AILTH, low >> 8);
	i2c_bus_write_byte(sens->i2c_dev, APDS9960_AIHTL, high & 0xFF);
	return i2c_bus_write_byte(sens->i2c_dev, APDS9960_AIHTH, high >> 8);
}

esp_err_t apds9960_enable_proximity_interrupt(apds9960_handle_t sensor, bool en)
{
	esp_err_t ret;
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	sens->_enable_t.pien = en;
	ret = i2c_bus_write_byte(sens->i2c_dev, APDS9960_MODE_ENABLE,
							 ((sens->_enable_t.gen << 6) | (sens->_enable_t.pien << 5) | (sens->_enable_t.aien << 4) | (sens->_enable_t.wen << 3) | (sens->_enable_t.pen << 2) | (sens->_enable_t.aen << 1) | sens->_enable_t.pon));
	apds9960_clear_interrupt(sensor);
	return ret;
}

uint8_t apds9960_read_proximity(apds9960_handle_t sensor)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	uint8_t data;

	if (i2c_bus_read_byte(sens->i2c_dev, APDS9960_PDATA, &data))
	{
		return ESP_FAIL;
	}

	return data;
}

esp_err_t apds9960_set_proximity_interrupt_threshold(apds9960_handle_t sensor,
													 uint8_t low, uint8_t high, uint8_t persistance)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	i2c_bus_write_byte(sens->i2c_dev, APDS9960_PILT, low);
	i2c_bus_write_byte(sens->i2c_dev, APDS9960_PIHT, high);

	if (persistance > 7)
	{
		persistance = 7;
	}

	sens->_pers_t.ppers = persistance;
	return i2c_bus_write_byte(sens->i2c_dev, APDS9960_PERS,
							  (sens->_pers_t.ppers << 4) | sens->_pers_t.apers);
}

bool apds9960_get_proximity_interrupt(apds9960_handle_t sensor)
{
	uint8_t data;
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	i2c_bus_read_byte(sens->i2c_dev, APDS9960_STATUS, &data);
	apds9960_set_status(sensor, data);
	return sens->_status_t.pint;
}

esp_err_t apds9960_clear_interrupt(apds9960_handle_t sensor)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	esp_err_t ret = i2c_bus_write_byte(sens->i2c_dev, NULL_I2C_MEM_ADDR,
									   APDS9960_AICLEAR);
	return ret;
}

esp_err_t apds9960_enable(apds9960_handle_t sensor, bool en)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	sens->_enable_t.pon = en;
	return i2c_bus_write_byte(sens->i2c_dev, APDS9960_MODE_ENABLE,
							  ((sens->_enable_t.gen << 6) | (sens->_enable_t.pien << 5) | (sens->_enable_t.aien << 4) | (sens->_enable_t.wen << 3) | (sens->_enable_t.pen << 2) | (sens->_enable_t.aen << 1) | sens->_enable_t.pon));
}

esp_err_t apds9960_set_gesture_dimensions(apds9960_handle_t sensor,
										  uint8_t dims)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	sens->_gconf3_t.gdims = dims;
	return i2c_bus_write_byte(sens->i2c_dev, APDS9960_GCONF3,
							  sens->_gconf3_t.gdims);
}

esp_err_t apds9960_set_light_intlow_threshold(apds9960_handle_t sensor,
											  uint16_t threshold)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	uint8_t val_low;
	uint8_t val_high;
	/* Break 16-bit threshold into 2 8-bit values */
	val_low = threshold & 0x00FF;
	val_high = (threshold & 0xFF00) >> 8;

	/* Write low byte */
	if (i2c_bus_write_byte(sens->i2c_dev, APDS9960_AILTL, val_low) != ESP_OK)
	{
		return ESP_FAIL;
	}

	/* Write high byte */
	return i2c_bus_write_byte(sens->i2c_dev, APDS9960_AILTH, val_high);
}

esp_err_t apds9960_set_light_inthigh_threshold(apds9960_handle_t sensor,
											   uint16_t threshold)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	uint8_t val_low;
	uint8_t val_high;
	/* Break 16-bit threshold into 2 8-bit values */
	val_low = threshold & 0x00FF;
	val_high = (threshold & 0xFF00) >> 8;

	/* Write low byte */
	if (i2c_bus_write_byte(sens->i2c_dev, APDS9960_AIHTL, val_low) != ESP_OK)
	{
		return ESP_FAIL;
	}

	/* Write high byte */
	return i2c_bus_write_byte(sens->i2c_dev, APDS9960_AIHTH, val_high);
}

esp_err_t apds9960_set_gesture_fifo_threshold(apds9960_handle_t sensor,
											  uint8_t thresh)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	sens->_gconf1_t.gfifoth = thresh;
	return i2c_bus_write_byte(sens->i2c_dev, APDS9960_GCONF1,
							  ((sens->_gconf1_t.gfifoth << 7) | (sens->_gconf1_t.gexmsk << 5) | sens->_gconf1_t.gexpers));
}

esp_err_t apds9960_set_gesture_waittime(apds9960_handle_t sensor,
										apds9960_gwtime_t time)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	uint8_t val;

	/* Read value from GCONF2 register */
	if (i2c_bus_read_byte(sens->i2c_dev, APDS9960_GCONF2, &val) != ESP_OK)
	{
		return ESP_FAIL;
	}

	/* Set bits in register to given value */
	time &= 0x07;
	val &= 0xf8;
	val |= time;
	/* Write register value back into GCONF2 register */
	return i2c_bus_write_byte(sens->i2c_dev, APDS9960_GCONF2, val);
}

esp_err_t apds9960_set_gesture_gain(apds9960_handle_t sensor,
									apds9960_ggain_t gain)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	sens->_gconf2_t.ggain = gain;
	return i2c_bus_write_byte(sens->i2c_dev, APDS9960_GCONF2,
							  ((sens->_gconf2_t.ggain << 5) | (sens->_gconf2_t.gldrive << 3) | sens->_gconf2_t.gwtime));
}

esp_err_t apds9960_set_gesture_LEDDrive(apds9960_handle_t sensor,
										apds9960_ggain_t drive)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;

	uint8_t val;

	/* Read value from GCONF2 register */
	if (i2c_bus_read_byte(sens->i2c_dev, APDS9960_GCONF2, &val) != ESP_OK)
	{
		return ESP_FAIL;
	}

	/* Set bits in register to given value */
	drive &= 0b00000011;
	drive = drive << 3;
	val &= 0b11100111;
	val |= drive;

	return i2c_bus_write_byte(sens->i2c_dev, APDS9960_GCONF2, val);
}

esp_err_t apds9960_set_gesture_proximity_threshold(apds9960_handle_t sensor,
												   uint8_t entthresh, uint8_t exitthresh)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	esp_err_t ret;

	if (i2c_bus_write_byte(sens->i2c_dev, APDS9960_GPENTH, entthresh))
	{
		return ESP_FAIL;
	}

	ret = i2c_bus_write_byte(sens->i2c_dev, APDS9960_GEXTH, exitthresh);
	return ret;
}
/* Container for gesture data */
typedef struct gesture_data_type
{
	uint8_t u_data[32];
	uint8_t d_data[32];
	uint8_t l_data[32];
	uint8_t r_data[32];
	uint8_t index;
	uint8_t total_gestures;
	uint8_t in_threshold;
	uint8_t out_threshold;
} gesture_data_type;

gesture_data_type gesture_data_;

esp_err_t apds9960_set_gesture_offset(apds9960_handle_t sensor,
									  uint8_t offset_up, uint8_t offset_down, uint8_t offset_left,
									  uint8_t offset_right)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	i2c_bus_write_byte(sens->i2c_dev, APDS9960_GOFFSET_U, offset_up);
	i2c_bus_write_byte(sens->i2c_dev, APDS9960_GOFFSET_D, offset_down);
	i2c_bus_write_byte(sens->i2c_dev, APDS9960_GOFFSET_L, offset_left);
	i2c_bus_write_byte(sens->i2c_dev, APDS9960_GOFFSET_R, offset_right);
	return ESP_OK;
}
#define FIFO_PAUSE_TIME 30 // Wait period (ms) between FIFO reads
#define DEBUG 1
/* State definitions */
enum
{
	NA_STATE,
	NEAR_STATE,
	FAR_STATE,
	ALL_STATE
};

/* Direction definitions */
enum
{
	DIR_NONE,
	DIR_LEFT,
	DIR_RIGHT,
	DIR_UP,
	DIR_DOWN,
	DIR_NEAR,
	DIR_FAR,
	DIR_ALL
};

int gesture_ud_delta_;
int gesture_lr_delta_;
int gesture_ud_count_;
int gesture_lr_count_;
int gesture_near_count_;
int gesture_far_count_;
int gesture_state_;
uint8_t gesture_motion_;

uint8_t gesture_disable_g = 0;
bool processGestureData(void)
{
	uint8_t u_first = 1;
	uint8_t d_first = 1;
	uint8_t l_first = 1;
	uint8_t r_first = 1;
	uint8_t u_last = 1;
	uint8_t d_last = 1;
	uint8_t l_last = 1;
	uint8_t r_last = 1;
	int ud_ratio_first;
	int lr_ratio_first;
	int ud_ratio_last;
	int lr_ratio_last;
	int ud_delta;
	int lr_delta;
	int i;

	/* If we have less than 4 total gestures, that's not enough */
	if (gesture_data_.total_gestures <= 4)
	{
		return false;
	}

	/* Check to make sure our data isn't out of bounds */
	if ((gesture_data_.total_gestures <= 32) &&
		(gesture_data_.total_gestures > 0))
	{

		/* Find the first value in U/D/L/R above the threshold */
		for (i = 0; i < gesture_data_.total_gestures; i++)
		{
			if (((gesture_data_.u_data[i] > GESTURE_THRESHOLD_OUT) &&
				 (gesture_data_.d_data[i] > GESTURE_THRESHOLD_OUT)) ||
				((gesture_data_.l_data[i] > GESTURE_THRESHOLD_OUT) &&
				 (gesture_data_.r_data[i] > GESTURE_THRESHOLD_OUT)))
			{

				u_first = gesture_data_.u_data[i];
				d_first = gesture_data_.d_data[i];
				l_first = gesture_data_.l_data[i];
				r_first = gesture_data_.r_data[i];
				break;
			}
		}

		/* Find the last value in U/D/L/R above the threshold */
		for (i = gesture_data_.total_gestures - 1; i >= 0; i--)
		{
			// #if DEBUG
			//             Serial.print(F("Finding last: "));
			//             Serial.print(F("U:"));
			//             Serial.print(gesture_data_.u_data[i]);
			//             Serial.print(F(" D:"));
			//             Serial.print(gesture_data_.d_data[i]);
			//             Serial.print(F(" L:"));
			//             Serial.print(gesture_data_.l_data[i]);
			//             Serial.print(F(" R:"));
			//             Serial.println(gesture_data_.r_data[i]);
			// #endif
			if (((gesture_data_.u_data[i] > GESTURE_THRESHOLD_OUT) &&
				 (gesture_data_.d_data[i] > GESTURE_THRESHOLD_OUT)) ||
				((gesture_data_.l_data[i] > GESTURE_THRESHOLD_OUT) &&
				 (gesture_data_.r_data[i] > GESTURE_THRESHOLD_OUT)))
			{

				u_last = gesture_data_.u_data[i];
				d_last = gesture_data_.d_data[i];
				l_last = gesture_data_.l_data[i];
				r_last = gesture_data_.r_data[i];
				break;
			}
		}
	}

	/* Calculate the first vs. last ratio of up/down and left/right */
	ud_ratio_first = ((u_first - d_first) * 100) / (u_first + d_first);
	lr_ratio_first = ((l_first - r_first) * 100) / (l_first + r_first);
	ud_ratio_last = ((u_last - d_last) * 100) / (u_last + d_last);
	lr_ratio_last = ((l_last - r_last) * 100) / (l_last + r_last);

	// #if DEBUG
	// 	Serial.print(F("Last Values: "));
	// 	Serial.print(F("U:"));
	// 	Serial.print(u_last);
	// 	Serial.print(F(" D:"));
	// 	Serial.print(d_last);
	// 	Serial.print(F(" L:"));
	// 	Serial.print(l_last);
	// 	Serial.print(F(" R:"));
	// 	Serial.println(r_last);

	// 	Serial.print(F("Ratios: "));
	// 	Serial.print(F("UD Fi: "));
	// 	Serial.print(ud_ratio_first);
	// 	Serial.print(F(" UD La: "));
	// 	Serial.print(ud_ratio_last);
	// 	Serial.print(F(" LR Fi: "));
	// 	Serial.print(lr_ratio_first);
	// 	Serial.print(F(" LR La: "));
	// 	Serial.println(lr_ratio_last);
	// #endif

	/* Determine the difference between the first and last ratios */
	ud_delta = ud_ratio_last - ud_ratio_first;
	lr_delta = lr_ratio_last - lr_ratio_first;

#if DEBUG
	// printf("Deltas: \r\n");
	// printf("UD: %d\r\n", ud_delta);
	// printf("LR: %d\r\n", lr_delta);
#endif

	/* Accumulate the UD and LR delta values */
	gesture_ud_delta_ += ud_delta;
	gesture_lr_delta_ += lr_delta;

#if DEBUG
	// printf("Accumulations: \r\n");
	// printf("UD: %d\r\n", gesture_ud_delta_);
	// printf("LR: %d\r\n", gesture_lr_delta_);
#endif

	/* Determine U/D gesture */
	if (gesture_ud_delta_ >= GESTURE_SENSITIVITY_1)
	{
		gesture_ud_count_ = 1;
	}
	else if (gesture_ud_delta_ <= -GESTURE_SENSITIVITY_1)
	{
		gesture_ud_count_ = -1;
	}
	else
	{
		gesture_ud_count_ = 0;
	}

	/* Determine L/R gesture */
	if (gesture_lr_delta_ >= GESTURE_SENSITIVITY_1)
	{
		gesture_lr_count_ = 1;
	}
	else if (gesture_lr_delta_ <= -GESTURE_SENSITIVITY_1)
	{
		gesture_lr_count_ = -1;
	}
	else
	{
		gesture_lr_count_ = 0;
	}

	/* Determine Near/Far gesture */
	if ((gesture_ud_count_ == 0) && (gesture_lr_count_ == 0))
	{
		if ((abs(ud_delta) < GESTURE_SENSITIVITY_2) &&
			(abs(lr_delta) < GESTURE_SENSITIVITY_2))
		{

			if ((ud_delta == 0) && (lr_delta == 0))
			{
				gesture_near_count_++;
			}
			else if ((ud_delta != 0) || (lr_delta != 0))
			{
				gesture_far_count_++;
			}

			if ((gesture_near_count_ >= 10) && (gesture_far_count_ >= 2))
			{
				if ((ud_delta == 0) && (lr_delta == 0))
				{
					gesture_state_ = NEAR_STATE;
				}
				else if ((ud_delta != 0) && (lr_delta != 0))
				{
					gesture_state_ = FAR_STATE;
				}
				return true;
			}
		}
	}
	else
	{
		if ((abs(ud_delta) < GESTURE_SENSITIVITY_2) &&
			(abs(lr_delta) < GESTURE_SENSITIVITY_2))
		{

			if ((ud_delta == 0) && (lr_delta == 0))
			{
				gesture_near_count_++;
			}

			if (gesture_near_count_ >= 10)
			{
				gesture_ud_count_ = 0;
				gesture_lr_count_ = 0;
				gesture_ud_delta_ = 0;
				gesture_lr_delta_ = 0;
			}
		}
	}

#if DEBUG

	// printf("UD_CT: %d\r\n", gesture_ud_count_);
	// printf("LR_CT: %d\r\n", gesture_lr_count_);
	// printf("NEAR_CT: %d\r\n", gesture_near_count_);
	// printf("FAR_CT: %d\r\n", gesture_far_count_);
	// printf("----------\r\n");

#endif

	return false;
}

bool decodeGesture()
{
	/* Return if near or far event is detected */
	if (gesture_state_ == NEAR_STATE)
	{
		gesture_motion_ = APDS9960_NEAR;
		return true;
	}
	else if (gesture_state_ == FAR_STATE)
	{
		gesture_motion_ = APDS9960_FAR;
		return true;
	}

	/* Determine swipe direction */
	if ((gesture_ud_count_ == -1) && (gesture_lr_count_ == 0))
	{
		gesture_motion_ = APDS9960_UP;
	}
	else if ((gesture_ud_count_ == 1) && (gesture_lr_count_ == 0))
	{
		gesture_motion_ = APDS9960_DOWN;
	}
	else if ((gesture_ud_count_ == 0) && (gesture_lr_count_ == 1))
	{
		gesture_motion_ = APDS9960_RIGHT;
	}
	else if ((gesture_ud_count_ == 0) && (gesture_lr_count_ == -1))
	{
		gesture_motion_ = APDS9960_LEFT;
	}
	else if ((gesture_ud_count_ == -1) && (gesture_lr_count_ == 1))
	{
		if (abs(gesture_ud_delta_) > abs(gesture_lr_delta_))
		{
			gesture_motion_ = APDS9960_UP;
		}
		else
		{
			gesture_motion_ = APDS9960_RIGHT;
		}
	}
	else if ((gesture_ud_count_ == 1) && (gesture_lr_count_ == -1))
	{
		if (abs(gesture_ud_delta_) > abs(gesture_lr_delta_))
		{
			gesture_motion_ = APDS9960_DOWN;
		}
		else
		{
			gesture_motion_ = APDS9960_LEFT;
		}
	}
	else if ((gesture_ud_count_ == -1) && (gesture_lr_count_ == -1))
	{
		if (abs(gesture_ud_delta_) > abs(gesture_lr_delta_))
		{
			gesture_motion_ = APDS9960_UP;
		}
		else
		{
			gesture_motion_ = APDS9960_LEFT;
		}
	}
	else if ((gesture_ud_count_ == 1) && (gesture_lr_count_ == 1))
	{
		if (abs(gesture_ud_delta_) > abs(gesture_lr_delta_))
		{
			gesture_motion_ = APDS9960_DOWN;
		}
		else
		{
			gesture_motion_ = APDS9960_RIGHT;
		}
	}
	else
	{
		return false;
	}

	return true;
}

void resetGestureParameters()
{
	gesture_data_.index = 0;
	gesture_data_.total_gestures = 0;

	gesture_ud_delta_ = 0;
	gesture_lr_delta_ = 0;

	gesture_ud_count_ = 0;
	gesture_lr_count_ = 0;

	gesture_near_count_ = 0;
	gesture_far_count_ = 0;

	gesture_state_ = 0;
	gesture_motion_ = APDS9960_NONE;
}
void print_gesture(uint8_t gesture)
{
	if (gesture == APDS9960_DOWN)
	{
		ESP_LOGE("APDS9960", "_DOWN");
	}
	else if (gesture == APDS9960_UP)
	{
		ESP_LOGE("APDS9960", "_UP");
	}
	else if (gesture == APDS9960_LEFT)
	{
		ESP_LOGE("APDS9960", "_LEFT");
	}
	else if (gesture == APDS9960_RIGHT)
	{
		ESP_LOGE("APDS9960", "_RIGHT");
	}
	else if (gesture == APDS9960_FAR)
	{
		ESP_LOGE("APDS9960", "_FAR");
	}
	else if (gesture == APDS9960_NEAR)
	{
		ESP_LOGE("APDS9960", "_NEAR");
	}
}

uint8_t apds9960_read_gesture(apds9960_handle_t sensor)
{
	uint8_t fifo_level = 0;
	uint8_t fifo_data[128];
	uint8_t gstatus;
	int bytes_read = 0;
	uint8_t motion;
	uint8_t use_histo = 0;
	int i;
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	unsigned long taskTick_counter = xTaskGetTickCount();

	if (gesture_disable_g)
	{

		ESP_LOGW("apds9960", "FLUSHING AFTER DISABLED");
		// flush
		taskTick_counter = xTaskGetTickCount();
		while (1)
		{
			vTaskDelay(FIFO_PAUSE_TIME / portTICK_RATE_MS);
			if (apds9960_gesture_valid(sensor))
			{

				/* Read the current FIFO level */
				i2c_bus_read_byte(sens->i2c_dev, APDS9960_GFLVL, &fifo_level);
#if DEBUG
				// printf("fifo_level %u\r\n", fifo_level);
#endif

				/* If there's stuff in the FIFO, read it into our data block */
				if (fifo_level > 0)
				{
					memset(fifo_data, 0, sizeof(fifo_data));
					i2c_bus_read_bytes(sens->i2c_dev, APDS9960_GFIFO_U, (fifo_level * 4), (uint8_t *)fifo_data);
				}
				// if ((xTaskGetTickCount() - taskTick_counter) > (500 / portTICK_RATE_MS))
				// 	break;
			}
			else
				break;
		}
		resetGestureParameters();
		apds9960_gesture_init(sensor);

		gesture_disable_g = 0;

		ESP_LOGW("apds9960", "DISABLED FLUSHED");
		return APDS9960_NONE;
	}

	/* Make sure that power and gesture is on and data is valid */
	if (!apds9960_gesture_valid(sensor))
	{
		apds9960_gesture_init(sensor);
		return APDS9960_NONE;
	}

	uint8_t histo[7];
	memset(histo, 0, sizeof(histo));

	/* Keep looping as long as gesture data is valid */
	resetGestureParameters();
	taskTick_counter = xTaskGetTickCount();

	while (1)
	{

		/* Wait some time to collect next batch of FIFO data */

		vTaskDelay(FIFO_PAUSE_TIME / portTICK_RATE_MS);

		/* If we have valid data, read in FIFO */
		if (apds9960_gesture_valid(sensor))
		{

			/* Read the current FIFO level */
			i2c_bus_read_byte(sens->i2c_dev, APDS9960_GFLVL, &fifo_level);
#if DEBUG
			// printf("fifo_level %u\r\n", fifo_level);
#endif

			/* If there's stuff in the FIFO, read it into our data block */
			if (fifo_level > 0)
			{
				memset(fifo_data, 0, sizeof(fifo_data));
				i2c_bus_read_bytes(sens->i2c_dev, APDS9960_GFIFO_U, (fifo_level * 4), (uint8_t *)fifo_data);
				bytes_read = (fifo_level * 4);
#if DEBUG
				// printf("FIFO Dump: ");
				// for (i = 0; i < bytes_read; i++)
				// {
				// 	printf("%u ", fifo_data[i]);
				// }
				// printf("\r\n");
#endif

				/* If at least 1 set of data, sort the data into U/D/L/R */
				if (bytes_read >= 4)
				{
					for (i = 0; i < bytes_read; i += 4)
					{
						gesture_data_.u_data[gesture_data_.index] =
							fifo_data[i + 0];
						gesture_data_.d_data[gesture_data_.index] =
							fifo_data[i + 1];
						gesture_data_.l_data[gesture_data_.index] =
							fifo_data[i + 2];
						gesture_data_.r_data[gesture_data_.index] =
							fifo_data[i + 3];
						gesture_data_.index++;
						gesture_data_.total_gestures++;
					}

#if DEBUG
					// printf("Up Data: ");
					// for (i = 0; i < gesture_data_.total_gestures; i++)
					// {
					// 	printf("%u ", gesture_data_.u_data[i]);
					// }
					// printf("\r\n");
#endif

					/* Filter and process gesture data. Decode near/far state */
					if (processGestureData())
					{
						if (decodeGesture())
						{
							use_histo = 1;
							if (gesture_motion_ < 7)
								if (histo[gesture_motion_] < 255)
									histo[gesture_motion_]++;
#if DEBUG
							//print_gesture(gesture_motion_);
#endif
						}
					}

					/* Reset data */
					gesture_data_.index = 0;
					gesture_data_.total_gestures = 0;
				}
			}
			if (((xTaskGetTickCount() - taskTick_counter) > (2000 / portTICK_RATE_MS)))
			{

				ESP_LOGW("apds9960", "TIMEOUT ,flushing and discarting all");
				// flush
				taskTick_counter = xTaskGetTickCount();
				while (1)
				{
					vTaskDelay(FIFO_PAUSE_TIME / portTICK_RATE_MS);
					if (apds9960_gesture_valid(sensor))
					{

						/* Read the current FIFO level */
						i2c_bus_read_byte(sens->i2c_dev, APDS9960_GFLVL, &fifo_level);
#if DEBUG
						// printf("fifo_level %u\r\n", fifo_level);
#endif

						/* If there's stuff in the FIFO, read it into our data block */
						if (fifo_level > 0)
						{
							memset(fifo_data, 0, sizeof(fifo_data));
							i2c_bus_read_bytes(sens->i2c_dev, APDS9960_GFIFO_U, (fifo_level * 4), (uint8_t *)fifo_data);
						}
						// if ((xTaskGetTickCount() - taskTick_counter) > (500 / portTICK_RATE_MS))
						// 	break;
					}
					else
						break;
				}

				resetGestureParameters();
				apds9960_gesture_init(sensor);

				ESP_LOGW("apds9960", "TIMEOUT ,FLUSHED");
				return APDS9960_NONE;
			}
		}
		else
		{

			/* Determine best guessed gesture and clean up */
			vTaskDelay(FIFO_PAUSE_TIME / portTICK_RATE_MS);
			if (!use_histo)
			{
				decodeGesture();
				motion = gesture_motion_;
			}
			else
			{
				motion = 0;
				uint8_t max = 0;
				for (i = 0; i < 7; i++)
				{
					if (histo[i] > max)
					{
						motion = i;
						max = histo[i];
					}
				}
			}

#if DEBUG
			// printf("END:\r\n");
			// print_gesture(motion);
#endif
			resetGestureParameters();
			apds9960_gesture_init(sensor);
			return motion;
		}
	}
}
uint8_t apds9960_read_gesture2(apds9960_handle_t sensor)
{
	uint8_t toRead;
	uint8_t buf[256];
	unsigned long taskTick_counter = 0;
	uint8_t gestureReceived;
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	uint8_t scan_counter = 0;
	uint8_t far_count = 0;
	uint8_t near_count = 0;

	while (1)
	{
		int up_down_diff = 0;
		int left_right_diff = 0;
		gestureReceived = 0;
		//		ESP_LOGI(":", "......");
		if (!apds9960_gesture_valid(sensor) || scan_counter > 10)
		{
			gestureReceived = APDS9960_NONE;
			//			ESP_LOGI(":", "sen_gvalid[%d]  scan_counter[%d]  ",
			//					sens->_gstatus_t.gvalid, scan_counter);
			apds9960_reset_counts(sensor);
			apds9960_gesture_init(sensor);

			return gestureReceived;
		}

		vTaskDelay(10 / portTICK_RATE_MS);
		i2c_bus_read_byte(sens->i2c_dev, APDS9960_GFLVL, &toRead);
		i2c_bus_read_bytes(sens->i2c_dev, APDS9960_GFIFO_U, toRead, buf);

		ESP_LOGW(":", "toRead %u", toRead);
#ifdef DEBUG_APDS9960
		ESP_LOGE(".", "raw values  for sacan(%d)", (scan_counter + 1));
		ESP_LOGI(":", "UP[%d]  DOWN[%d]  RIGHT[%d]  LEFT[%d]", buf[0], buf[1],
				 buf[2], buf[3]);
#endif
		if (abs((int)buf[0] - (int)buf[1]) > 13)
		{
			up_down_diff += (int)buf[0] - (int)buf[1];
#ifdef DEBUG_APDS9960
			ESP_LOGI(":", "up_down_diff = %d", up_down_diff);
#endif
		}

		if (abs((int)buf[2] - (int)buf[3]) > 13)
		{
			left_right_diff += (int)buf[2] - (int)buf[3];
#ifdef DEBUG_APDS9960
			ESP_LOGI(":", "left_right_diff = %d", left_right_diff);
#endif
		}
		/*
				if (abs((int)buf[0] - (int)buf[1]) > 13)
				{
					up_down_diff += (int)buf[0] - (int)buf[1];
		#ifdef DEBUG_APDS9960
					ESP_LOGI(":", "up_down_diff = %d", up_down_diff);
		#endif
				}

				if (abs((int)buf[2] - (int)buf[3]) > 13)
				{
					left_right_diff += (int)buf[2] - (int)buf[3];
		#ifdef DEBUG_APDS9960
					ESP_LOGI(":", "left_right_diff = %d", left_right_diff);
		#endif
				}
		#ifdef DEBUG_APDS9960
				ESP_LOGE(".", "----------------");
				ESP_LOGI(":", "up_down_diff=%d  	left_right_diff=%d", up_down_diff,
						 left_right_diff);
				ESP_LOGI(":", "up_cnt=%d  			down_cnt=%d", sens->up_cnt,
						 sens->down_cnt);
				ESP_LOGI(":", "left_cnt=%d  		right_cnt=%d", sens->left_cnt,
						 sens->right_cnt);
				ESP_LOGI(":", "far_count=%d  		near_count=%d", far_count, near_count);
		#endif

				if (up_down_diff != 0)
				{

					if (up_down_diff < 0)
					{

						if (sens->down_cnt > 0)
						{

							gestureReceived = APDS9960_UP;
						}
						else
						{
							//					ESP_LOGI("*", "up_cnt++");
							sens->up_cnt++;
						}
					}
					else if (up_down_diff > 0)
					{

						if (sens->up_cnt > 0)
						{

							gestureReceived = APDS9960_DOWN;
						}
						else
						{
							//					ESP_LOGI("*", "down_cnt++");
							sens->down_cnt++;
						}
					}
				}

				if (left_right_diff != 0)
				{
					if (left_right_diff < 0)
					{
						if (sens->right_cnt > 0)
						{
							gestureReceived = APDS9960_LEFT;
						}
						else
						{
							//					ESP_LOGI("*", "lef++");
							sens->left_cnt++;
						}
					}
					else if (left_right_diff > 0)
					{
						if (sens->left_cnt > 0)
						{
							gestureReceived = APDS9960_RIGHT;
						}
						else
						{
							//					ESP_LOGI("*", "right++");
							sens->right_cnt++;
						}
					}
				}

				// check far and near gesture
				if ((((int)buf[0] - (int)buf[1]) < 50) && (left_right_diff < 30))
					near_count++;
				else
					far_count++;

				if ((gestureReceived == 0) && (sens->down_cnt > 3) && (far_count >= 3) && (buf[0] < 200) & (buf[1] < 200))
					gestureReceived = APDS9960_FAR;

				if ((gestureReceived == 0) && (near_count >= 5) && (buf[0] > 200) && (buf[1] > 200))
					gestureReceived = APDS9960_NEAR;

					*/

		ESP_LOGW(":", "up_down_diff = %d	left_right_diff = %d", up_down_diff, left_right_diff);
		// if (up_down_diff != 0 || left_right_diff != 0 || near_count >= 1 || far_count >= 1)
		if (up_down_diff != 0 || left_right_diff != 0)
		{
			taskTick_counter = xTaskGetTickCount();
		}

		if ((left_right_diff > 30) && (abs(up_down_diff) < abs(left_right_diff)))
		{
			gestureReceived = APDS9960_RIGHT;
		}
		else if ((left_right_diff < -30) && (abs(up_down_diff) < abs(left_right_diff)))
		{
			gestureReceived = APDS9960_LEFT;
		}
		else
		{
			gestureReceived = APDS9960_NONE;
		}
		/*
		 #ifdef DEBUG_APDS9960
		ESP_LOGE(".", "new values  for sacan(%d)", (scan_counter + 1));
		ESP_LOGW(":", "up_down_diff=%d			left_right_diff=%d", up_down_diff,
				 left_right_diff);
		ESP_LOGW(":", "up_cnt=%d				down_cnt=%d", sens->up_cnt, sens->down_cnt);
		ESP_LOGW(":", "left_cnt=%d				right_cnt=%d", sens->left_cnt,
				 sens->right_cnt);
		ESP_LOGW(":", "far_count=%d				near_count=%d", far_count, near_count);
		 #endif
		 */
		if (gestureReceived || ((xTaskGetTickCount() - taskTick_counter) > (300 / portTICK_RATE_MS)))
		{

			apds9960_reset_counts(sensor);
			vTaskDelay(pdMS_TO_TICKS(200));
			apds9960_gesture_init(sensor);
			return gestureReceived;
		}
		scan_counter++;
	}
}

bool apds9960_gesture_valid(apds9960_handle_t sensor)
{

	uint8_t data;
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	i2c_bus_read_byte(sens->i2c_dev, APDS9960_GSTATUS, &data);
	sens->_gstatus_t.gfov = (data >> 1) & 0x01;
	sens->_gstatus_t.gvalid = data & 0x01;
	return sens->_gstatus_t.gvalid;
}

esp_err_t apds9960_set_gesture_pulse(apds9960_handle_t sensor,
									 apds9960_gpulselen_t gpulseLen, uint8_t pulses)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	sens->_gpulse_t.gplen = gpulseLen;
	sens->_gpulse_t.gpulse = pulses; // 10 pulses
	return i2c_bus_write_byte(sens->i2c_dev, APDS9960_GPULSE,
							  (sens->_gpulse_t.gplen << 6) | sens->_gpulse_t.gpulse);
}

esp_err_t apds9960_set_gesture_enter_thresh(apds9960_handle_t sensor,
											uint8_t threshold)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	esp_err_t ret;
	ret = i2c_bus_write_byte(sens->i2c_dev, APDS9960_GPENTH, threshold);
	return ret;
}

esp_err_t apds9960_set_gesture_exit_thresh(apds9960_handle_t sensor,
										   uint8_t threshold)
{
	apds9960_dev_t *sens = (apds9960_dev_t *)sensor;
	esp_err_t ret;
	ret = i2c_bus_write_byte(sens->i2c_dev, APDS9960_GEXTH, threshold);
	return ret;
}
