/**
 * @file deepdeck_tasks.h
 * @author ElectroNick (nick@dsd.dev)
 * @brief header file for the main FreeRtos tasks on DeepDeck
 * @version 0.1
 * @date 2022-12-08
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef INC_DEEPDECK_TASK_H
#define INC_DEEPDECK_TASK_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#ifdef OLED_ENABLE
extern TaskHandle_t xOledTask;
#endif
extern TaskHandle_t xKeyreportTask;


/**
 * @brief Tasks that manage the OLED
 * @todo Get a better way to handle the updates, so it does not refresh when is not needed.
 * 
 * @param pvParameters 
 */
void oled_task(void *pvParameters);

/**
 * @brief Tasks that manage the Gesture Sensor
 * @todo
 *
 * @param pvParameters
 */
void gesture_task(void *pvParameters);

/**
 * @brief Task that hanldes battery status and report to screen and BT.
 * 
 * @param pvParameters 
 */
void battery_reports(void *pvParameters);

/**
 * @brief Task that hanldes the key pressing of the mechanical keys
 * 
 * @param pvParameters 
 */
void key_reports(void *pvParameters);

void main_task(void *pvParameters);

/**
 * @brief Task that handles RGB lights
 * 
 * @param pvParameters 
 */
void rgb_leds_task(void *pvParameters);

/**
 * @brief Tasks that handle both encoders
 * 
 * @param pvParameters 
 */
void encoder_report(void *pvParameters);


/**
 * @brief Tasks that hanlde deep sleep mode
 * 
 * @param pvParameters 
 */
void deep_sleep(void *pvParameters);

#endif
