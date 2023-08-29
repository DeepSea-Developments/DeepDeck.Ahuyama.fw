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


#ifndef MENU_H
#define MENU_H

#include "esp_err.h"
#include "u8g2.h"
#include "oled_tasks.h"
#include "rotary_encoder.h"
#include "rgb_led.h"

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

#define MENU_CHAR_NUM 23

// List of return items that can be obtained in the menu functions
typedef enum 
{ 
    mret_none = -1,
    mret_exit = 0, 
    mret_goto_main_menu
} menu_ret;

typedef enum {
    MA_MENU,
    MA_FUNCTION,
    MA_END
} menu_item_action;

struct menu_str_t;
typedef struct menu_str_t menu_t;

typedef struct menu_item_str_t{
    char * description; //String of the menu item
    menu_item_action action;        //Action if the item. It can take to a new menu or run a function
    uint8_t next_menu;               // Pointer of the next menu
    menu_ret (*function_pointer)(void); // Pointer of a function
	
} menu_item_t;

struct menu_str_t{
    char * title;
    char * subtitle;
	menu_item_t * menu_item_array;
};



static deepdeck_status_t deepdeck_status;

uint8_t menu_selection(u8g2_t *u8g2, const char *title, uint8_t start_pos, const char *sl);

void menu_command(encoder_state_t encoder_action);

menu_event_t menu_get_event(void);

/** @brief Generate Splash Screen
 * */
void menu_screen(void);

void menu_init(void);



menu_ret menu_goto_sleep(void);
menu_ret menu_exit(void);

uint8_t menu_get_goto_sleep(void);
uint8_t menu_send_rgb_mode(uint8_t mode);
uint8_t menu_rgb_mode_0(void);
uint8_t menu_rgb_mode_1(void);
uint8_t menu_rgb_mode_2(void);
uint8_t menu_rgb_mode_3(void);
uint8_t menu_rgb_mode_4(void);

static menu_t menu_main;
extern menu_item_t m_main_array[];

extern menu_t menu_bluetooth; 
extern menu_item_t m_bluetooth_array[];

#endif
