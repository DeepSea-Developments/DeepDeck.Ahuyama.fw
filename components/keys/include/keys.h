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


// #include "key_definitions.h"
// #include "keyboard_config.h"

#define KEYS_Q_SIZE 20

// Define the queue handle
extern QueueHandle_t keys_q;

/**
 * @brief Enum of possible key event states.
 *
 */
typedef enum {
    /** Press event issued */
    KEY_PRESSED = 0,
    KEY_RELEASED,
    KEY_SHORT_P,
    KEY_LONG_P_TIMEOUT,
    KEY_LONG_P,
    KEY_TAP_DANCE,
    KEY_LEADER, 
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
} keys_event_struct_t;

/**
 * @brief Enum of keys modes.
 *
 */
typedef enum {
    /** Press event issued */
    RAW_MODE = 0,
    CLEAN_MODE,
} keys_modes_t;

#define ENABLE_V_TAP_DANCE  (1<<0)
#define ENABLE_V_LEADER_KEY (1<<1)
#define LONG_TRIGGER_AT_TIMEOUT (1<<0)

/**
 * @brief Structure that configure the mode of the component.
 *
 */
typedef struct {
    keys_modes_t mode;      /*!< Mode of the keys.
                                 - Raw mode sends the information of pressed and unpressed keys
                                 - Clean mode accepts short pressed, long pressed, tap dance and leader key */
    uint8_t enable_v;       /*!< Vector to enable or disable actions in Clean mode. in Raw mode it does not
                                 have any functionality.
                                 bit 0 - 1 to enable Tap dance action. Use ENABLE_V_TAP_DANCE definition.
                                 bit 1 - 1 to enable leader key action. Use ENABLE_V_LEADER_KEY definition.
                                  */
    uint8_t options;        /*!< Used to select additional options:
                                bit 0: 1 for selecting trigger at timeout, 0 for selecting trigger at unpress. */
    uint16_t long_time;     /*!< minimum time for considering a key pressed as KEY_LONG_P */
    uint16_t interval_time; /*!< time used for tapdance or leader key. if after a key is pressed, another key is
                                  not pressed before interval time ends, it will be consider the end of the action.*/
} keys_config_struct_t;

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
