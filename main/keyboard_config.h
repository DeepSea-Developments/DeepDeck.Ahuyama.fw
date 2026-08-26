#ifndef KEYBOARD_CONFIG_H
#define KEYBOARD_CONFIG_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include "driver/gpio.h"
#include "driver/touch_pad.h"
#include "driver/adc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "oled_tasks.h"

#define FIRMWARE_VERSION "0.5.7"
#define MODULE_ID "ESP32"
#define GATTS_TAG "Ahuyama" // The device's name
#define MAX_BT_DEVICENAME_LENGTH 40

/* Device identification, published over BLE in the Device Information Service
 * as the PnP ID characteristic (0x2A50). This is what a host reads to identify
 * the device: macOS surfaces it as Vendor ID / Product ID, and tools that bind
 * input to a particular device - Keyboard Maestro's device triggers, for one -
 * cannot save a binding when these read as zero.
 *
 * 0x1209 is the pid.codes vendor ID for open source hardware. 0x0001 under it
 * is the RESERVED TEST PID: intended for development, and explicitly not for
 * shipping firmware. Claim a product ID at https://pid.codes before release
 * and change DEEPDECK_PID here.
 */
#define DEEPDECK_VID_SOURCE 0x02   // 0x01 = Bluetooth SIG, 0x02 = USB-IF
#define DEEPDECK_VID 0x1209        // pid.codes
#define DEEPDECK_PID 0x0001        // TEST PID - claim a real one before release
#define DEEPDECK_PRODUCT_VERSION 0x0100
#define DEEPDECK_MANUFACTURER "DeepSea Developments"

#define MASTER  // undefine if you are not flashing the main controller
// #define SPLIT_MASTER	 // undefine if keyboard is not split and master
//#define SLAVE	 // undefine if keyboard is master

#define DEBOUNCE 7 //debounce time in ms

//Define matrix
#define KEYPADS 1 // intended in order to create a Multiple keypad split boards
#define MATRIX_ROWS 4
#define MATRIX_COLS 4 // For split keyboards, define columns for one side only.

#define LAYERS 3 // number of layers defined

// Select diode direction
#define COL2ROW
//#define ROW2COL

//Encoder definitions
#define R_ENCODER_1 // undefine if no rotary encoder is used
//#define R_ENCODER_SLAVE // undefine if no rotary encoder is used on slave pad
// #define ENCODER1_A_PIN GPIO_NUM_26 // encoder phase A pin
// #define ENCODER1_B_PIN GPIO_NUM_25// encoder phase B pin
// #define ENCODER1_S_PIN GPIO_NUM_34// encoder switch pin

#define ENCODER1_A_PIN GPIO_NUM_25 // encoder phase A pin
#define ENCODER1_B_PIN GPIO_NUM_26// encoder phase B pin
#define ENCODER1_S_PIN GPIO_NUM_34// encoder switch pin

#define ENCODER1_S_ACTIVE_LOW 0	  // encoder switch is active_low=1 active_high=0

#define R_ENCODER_2 // undefine if no rotary encoder is used
#define ENCODER2_A_PIN GPIO_NUM_33// encoder phase A pin
#define ENCODER2_B_PIN GPIO_NUM_32// encoder phase B pin
#define ENCODER2_S_PIN GPIO_NUM_27// encoder switch pin
#define ENCODER2_S_ACTIVE_LOW 0	  // encoder switch is active_low=1 active_high=0

#define RGB_LEDS

// Gesture sensor. Comment to disable
#define GESTURE_ENABLE

// Wifi. Comment to disable
#define WIFI_ENABLE

// MDNS. Comment to disable
// #define USE_MDNS

// Overwrite always Non Volatile Storage. When the memory is stored for the first time, it will not overwrite it from flashing.
// If yu want to change layers from code, not from the user interface, you have to either, erase flash
// every time you make a modification, or uncomment this line.
// #define LAYER_MODIFICATION_MODE

//OLED Parameters
#define OLED_ENABLE //undefine if no oled is used.
#define ROTATION LANDSCAPE
#define OLED_SDA_PIN GPIO_NUM_21
#define OLED_SCL_PIN GPIO_NUM_22

/*Battery monitoring
 * Please read check battery_monitor.h for resistor values before applying
 * use ADC1 only,  */

//#define BATT_STAT //define to enable battery monitoring
#define BATT_PIN ADC1_CHANNEL_7 //gpio pin 35, refer to the esp32 before modifying

//deep sleep parameters, mind that reconnecting after deep sleep might take a minute or two
//#define SLEEP_MINS 50 // undefine if you do not need deep sleep, otherwise define number of minutes for deepsleep

// Proximity wake. Reaching toward the pad brings the OLED back without pressing
// anything. Requires GESTURE_ENABLE, since it uses the same APDS-9960.
//
// Sampled ONLY while the screensaver has the panel blanked, which is the only
// time the answer is useful - so it costs nothing while you are actually using
// the keyboard, and cannot be confused by a hand resting on the keys.
//
// Measured on an Ahuyama: an empty desk reads 0-4 (99th percentile 4), a hand
// over the sensor reads 41-43. The threshold sits in that gap. Comment out
// PROXIMITY_WAKE to disable.
#define PROXIMITY_WAKE
#define PROXIMITY_WAKE_THRESHOLD 10

// Tuning aid, off by default. Uncomment to log every proximity reading at or
// above this value while the panel is blanked - which is how the numbers in the
// comment above were measured. Useful on different hardware, or if the sensor
// ends up behind a different cover, since the crosstalk floor depends on both.
// #define PROXIMITY_WAKE_DEBUG 3

// Biased toward false positives: waking when you were not reaching is a shrug,
// failing to wake when you were is the feature not working. Measured on an
// Ahuyama, at the driver's default 4x gain and 8 pulses:
//
//   empty desk   0-4 typically, 9 at the very worst (2 samples in 611)
//   hand at the moment it becomes visible   12-25
//   hand close   42
//
// 10 sits above the worst observed noise and below the weakest real detection,
// and two CONSECUTIVE samples are required on top of that.
//
// Raising PGAIN to 8x and the pulse count was tried and REVERTED: it lifted the
// noise floor from 3 to 12-18 while the hand signal only went 42 -> 104, which
// halved the signal-to-noise ratio and produced real false wakes. The floor is
// crosstalk - the sensor seeing its own LED reflected off the cover - so it
// scales with LED energy just as fast as the signal does. If more range is ever
// needed, the lever is the POFFSET_UR/POFFSET_DL registers (0x9D/0x9E, both
// default 0) to cancel that crosstalk first, NOT more gain.

// Screensaver. Blanks the OLED after this many SECONDS without a key press,
// knob movement or gesture. This is only the default: the timeout is adjustable
// from the OLED menu (Screensaver) and stored in NVS, where 0 means "never
// blank". Comment out to leave the screensaver task out of the build entirely.
#define SCREENSAVER_SECS 60


/*
 *---------------------------- Everything below here should not be modified for standard usage----------------------
 *
 * */
#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))
#define SET_BIT(var,pos) (var |= 1UL << pos);

#define MAX_LAYER (LAYERS-1)
#define MOD_LED_BYTES 2 //bytes for led status and modifiers
#define MACRO_LEN 5 //keys for macros
#define MACROS_NUM 40 //Available macro length
#define USER_MACROS_NUM 200
#define USER_MACRO_NAME_LEN 10

extern uint16_t macros[MACROS_NUM][MACRO_LEN];

#define KEYMAP_COLS MATRIX_COLS*KEYPADS  // used for a symmetrical split keyboard
#define REPORT_LEN (MOD_LED_BYTES+MACRO_LEN+MATRIX_ROWS*KEYMAP_COLS) //size of hid reports with NKRO and room for 3 key macro
#define REPORT_COUNT_BYTES (MATRIX_ROWS*KEYMAP_COLS+MACRO_LEN)

#define PLUGIN_BASE_VAL 0x135 // 0x135 default value. -->309
#define LAYER_HOLD_MAX_VAL 0x134 //0x134 default value. -->308
#define LAYER_HOLD_BASE_VAL 0x123 //0x123 default value. -->291
#define MACRO_BASE_VAL 500 //0x103 default value. -->259
#define MACRO_HOLD_MAX_VAL 600
#define MACRO_BASE_VAL_INT 259
#define LAYERS_BASE_VAL 0xFF // 0xFF default value. -->255

#define LAYER_ADJUST_MIN 400
#define LAYER_ADJUST_MAX 410


#define ENCODER_SIZE 5 //Number of actions that the encoder has
#define GESTURE_SIZE 6 //Number of actions gesture sensor has

typedef struct config_data {
	char bt_device_name[MAX_BT_DEVICENAME_LENGTH];
} config_data_t;

extern uint8_t current_layout;
extern uint8_t curr_led;

extern uint16_t default_encoder_map[LAYERS][ENCODER_SIZE];
extern uint16_t default_slave_encoder_map[LAYERS][ENCODER_SIZE];

#define MAX_LAYOUT_NAME_LENGTH 15
#define MAX_LAYOUT_NUMBER 10
// array to hold names of layouts for oled
extern char default_layout_names[LAYERS][MAX_LAYOUT_NAME_LENGTH];

/// @brief 
extern TaskHandle_t xKeyreportTask;



#define I2C_MASTER_SCL_IO           (gpio_num_t)22          /*!< gpio number for I2C master clock */
#define I2C_MASTER_SDA_IO           (gpio_num_t)21          /*!< gpio number for I2C master data  */
#define I2C_MASTER_NUM              I2C_NUM_0   /*!< I2C port number for master dev */
#define I2C_MASTER_TX_BUF_DISABLE   0           /*!< I2C master do not need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0           /*!< I2C master do not need buffer */
#define I2C_MASTER_FREQ_HZ          400000      /*!< I2C master clock frequency */


#define MEM_WIFI_TASK				1024*4
#define MEM_SLEEP_TASK				1024*4
#define MEM_SCREENSAVER_TASK		1024*4
#define	MEM_BATTERY_TASK			1024*4
#define MEM_KEYBOARD_TASK			1024*8
#define	MEM_LEDS_TASK				1024*4
#define MEM_ENCODER_TASK			1024*4
#define MEM_OLED_TASK				1024*4
#define MEM_GESTURE_TASK			1024*4

#define PRIOR_BASE					3
#define PRIOR_WIFI_TASK				4
#define PRIOR_SLEEP_TASK			2
#define PRIOR_BATTERY_TASK			3
#define PRIOR_KEYBOARD_TASK			3
#define PRIOR_LEDS_TASK				6
#define PRIOR_ENCODER_TASK			4
#define PRIOR_OLED_TASK				3
#define PRIOR_GESTURE_TASK			4
#define PRIOR_SCREENSAVER_TASK		2


#endif
//
