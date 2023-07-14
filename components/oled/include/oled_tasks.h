/*
 * oled_tasks.h
 *
 *  Created on: 01 Sep 2018
 *      Author: gal
 */

#ifndef OLED_TASKS_H_
#define OLED_TASKS_H_

#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/queue.h>
#include <esp_log.h>
#include "u8g2.h"

// rotation defines
#define LANDSCAPE U8G2_R0
#define DEG90 U8G2_R1
#define DEG180 U8G2_R2
#define DEG270 U8G2_R3

extern u8g2_t u8g2; // a structure which will contain all the data for one display

/** @brief Setup the oled screen
 * @param rotation - Set rotation
 * */
void init_oled(const u8g2_cb_t *rotation);

/** @brief deinitialize the oled screen
 * */

void deinit_oled(void);

/** @brief connecting waiting animation
 * */
void waiting_oled(void);

/** @brief connected graphic
 * */
void ble_connected_oled(void);

// /** @brief connected graphic
//  * */
// void ble_slave_oled(void);

/**
 * @brief
 *
 */
void wifi_connected_oled(char *ip_char);

/** @brief running oled task
 * */
void update_oled(void);

/** @brief Generate Splash Screen
 * */
void splashScreen(void);

/** @brief Generate Berlin Dance
 * */
void berlinDance(void);

/** @brief Queue for sending layer to oled
 **/
extern QueueHandle_t layer_recieve_q;

/** @brief Queue for sending led status to oled
 **/
extern QueueHandle_t led_recieve_q;

/**
 * Made with Marlin Bitmap Converter
 * https://marlinfw.org/tools/u8glib/converter.html
 *
 * This bitmap from the file 'DeepDeck.png'
 */

#define SPLASH_BMPWIDTH 128

#endif /* OLED_TASKS_H_ */
