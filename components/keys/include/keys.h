/**
 * @file keys.h
 * @author ElectroNick (nick@dsd.dev)
 * @brief Component that controls the mechanical switches (keys) that are disposed in a matrix way.
 * @date 2023-10-19
 * @copyright Copyright (c) 2023
 *
 * MIT License
 * Copyright (c) 2023
 *
 * DeepDeck, a product by DeepSea Developments.
 * More info on DeepDeck @ www.deepdeck.co
 * DeepseaDev.com
 *
 */

#ifndef KEYS_H
#define KEYS_H

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "keyboard_config.h"

// #include "key_definitions.h"


#define KEYS_Q_SIZE 20
#define KEYS_CONFIG_Q_SIZE 3

// Define the queue handle
extern QueueHandle_t keys_q;

// Define the queue handle
extern QueueHandle_t keys_config_q;

/**
 * @brief Enum of possible key event states.
 *
 */
typedef enum {
    /** Press event issued */
    KEY_RELEASED = 0,
    KEY_PRESSED,
    KEY_MT_SHORT,           //When ModTap enabled. Event when key is pressed in a short time.
    KEY_MT_LONG_TIMEOUT,    //When ModTap enabled. Timeout of long time. Useful when long tap is a modifier, like shift or control
    KEY_MT_LONG,            //When ModTap enabled. Event when key is unpressed after long time.
    KEY_TAP_DANCE,          //When TapDance enabled. Event should send how many taps where done. (1 to 255)
    KEY_LEADER 
} key_events_t;

/**
 * @brief Structure that notifies the events on the keyboard.
 *
 */
typedef struct {
    key_events_t event;     /*!< Possible event to be notified */
    uint8_t counter;        /*!< Counter to be used for tap dance or leader key */
    uint8_t key_pos;        /*!< Position of the key. Its a 1d array, so the get the position 
                                 in the matrix, ((ROW-1) * TOTAL_COLS)) + (COL-1)  
                                 multiply ROW*COL to get the index.
                                 In a 4x4 keyboard, this applies:
                                 K11 (key row 1, column 1, being at top left) is key_pos[0]
                                 K14 (key row 1, column 4) is key_pos[3]
                                 k41 (key row 4, column  ), is key_pos[12]
                                 */
    uint32_t time;          /*!< Time between the press event happen and the release. */
    uint8_t lk_seq_array[LK_MAX_KEYS]; /*!< When the keys are in leader key mode, itreturns the array
                                            of keys pressed. take into account that this array only stores
                                            up to LK_MAX_KEYS. You should take care of counter to see if 
                                            user pressed more than LK_MAX_KEYS */
} keys_event_struct_t;



#define MODE_V_TAPDANCE_ENABLE       (1 << 0)  // Will enable the TAPDANCE event
#define MODE_V_MODTAP_ENABLE         (1 << 1)  // Will enable the LONG_PRESSED_TIMEOUT and LONG_PRESSED event
#define MODE_V_LONG_P_SIMPLE         (1 << 2)  // Will only send the LONG_PRESSED action when the key is unpressed
                                        // This is not usefull for normal macro applications, but for other interactions.
#define MODE_V_RAW_DISABLE           (1 << 3) // This bit will disable sending the PRESSED event when the key is first pressed  

#define MODE_V_IS_TAPDANCE_ENABLED(mode_v_item)         (mode_v_item & MODE_V_TAPDANCE_ENABLE)
#define MODE_V_IS_MODTAP_ENABLED(mode_v_item)           (mode_v_item & MODE_V_MODTAP_ENABLE)
#define MODE_V_IS_LONG_P_SIMPLE_ENABLED(mode_v_item)    (mode_v_item & MODE_V_LONG_P_SIMPLE)
#define MODE_V_IS_RAW_DISABLED(mode_v_item)             (mode_v_item & MODE_V_RAW_DISABLE)

#define KEY_CONFIG_NORMAL_MODE      0
#define KEY_CONFIG_LEADERKEY_MODE   1

/**
 * @brief Structure that configure the mode of the component.
 *
 */
typedef struct {
    uint8_t mode_vector[MATRIX_ROWS*MATRIX_COLS];   /*!< Vector to enable or disable actions.
                                                    
                                                    default: 0x00 or "raw mode". Send the press and release as soon as it happens
                                                    bit 0 - 1 to enable Tap dance action. Use MODE_V_TAPDANCE_ENABLE definition.
                                                    bit 1 - 1 to enable Modtap action. Use MODE_V_MODTAP_ENABLE definition.
                                                    bit 2 - 1 to enable simple long press event (only event sent when key is unpressed)
                                                    bit 3 - 1 t disable sending the pressed event. 
                                                    */
    uint8_t general_mode;   /*!< KEY_CONFIG_NORMAL_MODE for normal mode. KEY_CONFIG_LEADERKEY_MODE for leader key mode*/
    uint16_t long_time;     /*!< minimum time for considering a key pressed as KEY_LONG_P */
    uint16_t interval_time; /*!< time used for tapdance or leader key. if after a key is pressed, another key is
                                  not pressed before interval time ends, it will be consider the end of the action.*/
} keys_config_struct_t;

/**
 * @brief Init the leaderkey config structure.
 * 
 * @param key_config config struct
 */
void leaderkey_config_struct_init(keys_config_struct_t * key_config);


/**
 * @brief Handler of the events on the keyboard.
 *
 */
typedef void (*KeyEventHandler)(keys_event_struct_t event);

/*
 * @brief deinitialize rtc pins
 */
void rtc_matrix_deinit(void);

/*
 * @brief initialize rtc pins
 */
void rtc_matrix_setup(void);

/*
 * @brief initialize matrix
 */
void matrix_setup(void);

/*
 * @brief scan matrix
 */
void scan_matrix(keys_config_struct_t keys_config, KeyEventHandler handler);

/*
 * @brief Init the key task in FreeRTOS.
 */
void init_keys_task(void);

#endif /* KEYS_H_ */
