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

#include <stdbool.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


#define TRUNC_SIZE 20

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
 * @brief Tasks that handles deep sleep mode
 * 
 * @param pvParameters 
 */
void deep_sleep(void *pvParameters);

/**
 * @brief Task that blanks the OLED after a period without input
 *
 * @param pvParameters
 */
void screensaver(void *pvParameters);

/**
 * @brief Reset the screensaver idle timer without changing the screen state
 *
 * Call this from anything that counts as user activity but must not disturb
 * the current display state.
 */
void screensaver_notify_activity(void);

/**
 * @brief Reset the idle timer and, if the screen is blanked, turn it back on
 *
 * @return true if the screensaver was active and has been dismissed
 */
bool screensaver_wake(void);

/**
 * @brief Is the screensaver currently blanking the panel?
 *
 * Lets other subsystems ask the question without needing the UI state enum.
 *
 * @return true while the panel is blanked
 */
bool screensaver_is_blanked(void);

/**
 * @brief Set the idle timeout in seconds. 0 disables the screensaver.
 *
 * Only changes the running value - persist it with nvs_save_screensaver_secs().
 */
void screensaver_set_timeout_sec(uint16_t seconds);

/**
 * @brief Current idle timeout in seconds (0 when the screensaver is off)
 */
uint16_t screensaver_get_timeout_sec(void);

#endif
