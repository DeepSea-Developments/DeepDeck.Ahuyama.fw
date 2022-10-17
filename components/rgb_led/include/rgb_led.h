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

#define RGB_LED_REFRESH_SPEED  (10)
#define KEYBOARD_RGB_GPIO       17
#define NOTIFICATION_RGB_GPIO   23
#define RGB_LED_KEYBOARD_NUMBER 16
#define RGB_LED_NOTIFICATION_NUMBER 2

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

//pulsating keys
rbg_key rgb_key_status[RGB_LED_KEYBOARD_NUMBER];


#endif /* RGB_LED_H_ */