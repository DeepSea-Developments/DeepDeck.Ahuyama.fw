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
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_err.h"
#include "u8g2.h"
#include "oled_tasks.h"
#include "rotary_encoder.h"

/** 
 * @brief DeepDeck Status 
 * 
 */
typedef enum {
	S_NORMAL = 0,
    S_SETTINGS
	
} deepdeck_status_t;

typedef enum {
    MENU_NONE,
	MENU_SELECT,
    MENU_HOME,
    MENU_UP,
    MENU_DOWN,
    MENU_PREV,
    MENU_NEXT
	
} menu_event_t;



deepdeck_status_t deepdeck_status;

uint8_t menu_selection(u8g2_t *u8g2, const char *title, uint8_t start_pos, const char *sl);

void menu_command(encoder_state_t encoder_action);

menu_event_t menu_get_event(void);

/** @brief Generate Splash Screen
 * */
void menu_screen(void);

#ifdef __cplusplus
}
#endif
