/**
 * @file gesture_handles.h
 * @author Mauro (mauriciop@dsd.dev)
 * @brief 
 * @version 0.1
 * @date Feb 2023
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef MAIN_GESTURE_HANDLES_H_
#define MAIN_GESTURE_HANDLES_H_

#include <stdio.h>
#include "esp_log.h"
#include "apds9960.h"

#include "nvs_funcs.h"
#include "key_definitions.h"
#include "keyboard_config.h"

//#define APDS9960_VL_IO                       (gpio_num_t)19

#define INTERRUPT_GPIO 19
#define APDS9960_INT_PIN     19
#define GPIO_INPUT_PIN_SEL  (1ULL<<APDS9960_INT_PIN)
#define ESP_INTR_FLAG_DEFAULT 0
#define LONG_TIME 0xffff
extern TaskHandle_t xGesture;
extern TaskHandle_t xOledTask;
extern SemaphoreHandle_t xSemaphore;
//extern dd_layer *key_layouts;

extern apds9960_handle_t apds9960;

//interrup callback to handle apds9960 pint interrup
void IRAM_ATTR gesture_isr_handler(void *arg);

//software timer to block gesture detection ehile using right rotary encoder
esp_err_t set_timer(void);



/**
 * @brief How to process encoder activity
 *
 * @param
 * @return
 *      - gesture_state_t: TODO
 */
void gesture_command(uint8_t command, uint16_t gesture_commands[GESTURE_SIZE]);



void apds9960_test_gesture();
// deinit apds9960 sensor - delete sensor for i2c bus
void apds9960_deinit();
// test to free apds9960 after Ole-menu-config
void apds9960_free();
// Config and startup  gesture sensor
//void apds9960_init();
void apds9960_init(i2c_bus_handle_t *i2cbus);
// read  apds9960 gesture
void read_gesture();
//configure interrup input pin for gesture detection
void config_interrup_pin(void);
void disable_interrup_pin(void);


#endif /* MAIN_GESTURE_HANDLES_H_ */
