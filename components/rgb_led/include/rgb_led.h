// Copyright 2020 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef RGB_LED_H_
#define RGB_LED_H_

#include "led_strip.h"
#include "esp_err.h"

/**
 * @brief Type of Rotary underlying device handle
 *
 */



#define RMT_TX_CHANNEL_KEYPAD RMT_CHANNEL_0
#define RMT_TX_CHANNEL_NOTIFICATION RMT_CHANNEL_1

#define RGB_LED_REFRESH_SPEED  (20) //default 30
#define KEYBOARD_RGB_GPIO       17
#define NOTIFICATION_RGB_GPIO   23
#define RGB_LED_KEYBOARD_NUMBER 16
#define RGB_LED_NOTIFICATION_NUMBER 2

/* Global LED brightness, as a percentage of full output. It is applied where
 * the colour reaches the strip rather than inside hsv2rgb(), so it dims every
 * mode - including the solid and per key modes, which write RGB values
 * straight through and never touch hsv2rgb() at all.
 *
 * The default is deliberately not 100: the strips are uncomfortably bright at
 * full output, which is what upstream PR #11 worked around by hard clamping
 * them. This is the same observation made adjustable instead. */
#define RGB_LED_BRIGHTNESS_DEFAULT 50

/* LED mode numbers. These are part of the /api/led contract and are stored in
 * NVS, so the values are fixed and may only be appended to. 6 and 7 are left
 * free for the fireball and rainbow effects in upstream PR #49, and 8 matches
 * the per key colour mode that PR uses, to keep a future merge cheap. */
enum
{
    RGB_MODE_OFF = 0,
    RGB_MODE_PULSATING = 1,
    RGB_MODE_PROGRESSIVE = 2,
    RGB_MODE_SPARKS = 3,
    RGB_MODE_SOLID = 4,
    RGB_MODE_SOLID_MAPPED = 5,
    RGB_MODE_KEY_COLOR = 8,
    RGB_MODE_LAYER_COLOR = 9,
};

/** @brief Queue for sending mouse reports
 * @see mouse_command_t */
extern QueueHandle_t keyled_q;

void rgb_notification_led_init(void);
void rgb_key_led_init(void);

void rgb_key_led_press(uint8_t row, uint8_t col);

void hsv2rgb(uint32_t h, uint32_t s, uint32_t v, uint32_t *r, uint32_t *g, uint32_t *b);

// Todo list:
// Integrate the LED modes into the configuration. Tasks:
// [] Review QMK way of manage leds with keys (mode, speed, color?)
// [] Create an structure that manage LEDs characteristics
// [] Create function that controls 2 mode LEDs
// [] Create way of changing LED characteristic via key definitios OR via embedded settings menu.
// 


void key_led_modes(void);

typedef struct rbg_key {
        uint16_t h;
        int8_t s;
        int8_t v;
    }rbg_key; 

typedef struct rgb_mode_t {
        uint8_t mode;
        uint8_t H;
        uint8_t S;
        uint8_t V;
        uint8_t speed;
        uint8_t rgb[3];
        uint8_t brightness;     // 0-100, scales the output of every mode

    }rgb_mode_t;

/**
 * @brief Fill led_mode with the compiled in defaults
 *
 * nvs_load_led_mode() only overwrites the fields that are actually present in
 * NVS, so on a device that has never had them saved it leaves the rest of the
 * struct untouched. Call this first and every field has a sane value whether
 * or not NVS has anything to say about it.
 *
 * @param led_mode
 */
void rgb_mode_defaults(rgb_mode_t *led_mode);

//pulsating keys
extern rbg_key rgb_key_status[RGB_LED_KEYBOARD_NUMBER];


#endif /* RGB_LED_H_ */