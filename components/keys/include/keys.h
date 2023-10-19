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
// #include "key_definitions.h"
// #include "keyboard_config.h"




void keys_hello_world(void);

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
    TAP_DANCE,
    LEADER_KEY, 
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
    uint16_t time;          /*!< Time between the press event happen and the release. */
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

#define ENABLE_V_TAP_DANCE  0x01
#define ENABLE_V_LEADER_KEY 0x02
#define LONG_TRIGGER_AT_TIMEOUT
#define LONG_TRIGGER_AT

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
void scan_matrix(uint8_t * current_matrix);


// /**
//  * @brief Type of Rotary underlying device handle
//  *
//  */
// typedef void *rotary_encoder_dev_t;

// /** 
//  * @brief State of the rotary encoder 
//  * 
//  */
// typedef enum {
//     /** Press event issued */
//     ENC_IDLE = 0,
//     ENC_UP,
//     ENC_DOWN,
//     ENC_BUT_SHORT_PRESS,
//     ENC_BUT_LONG_PRESS,
//     ENC_BUT_DOUBLE_PRESS, 
// } encoder_state_t;

// /** 
//  * @brief Finit state machine states for beign able to do 
//  * 
//  */
// typedef enum {
//     S_IDLE = 0,
//     S_BUTTON_PRESSED,
//     S_BUTTON_RELEASED,
//     S_LONG_PRESSED,
//     S_DOUBLE_PRESSED
 
// } encoder_fsm_t;

// /**
//  * @brief Type of rotary encoder configuration
//  *
//  */
// typedef struct {
//     rotary_encoder_dev_t dev; /*!< Underlying device handle */
//     int phase_a_gpio_num;     /*!< Phase A GPIO number */
//     int phase_b_gpio_num;     /*!< Phase B GPIO number */
//     int button_gpio_num;      /*!< Button GPIO number  */
//     int button_active_low;    /*!< Button ActiveLow=1 ActiveHigh=0=default */
//     int flags;                /*!< Extra flags */
// } rotary_encoder_config_t;



// /**
//  * @brief Type of rotary encoder handle
//  *
//  */
// typedef struct rotary_encoder_t rotary_encoder_t;

// /**
//  * @brief Rotary encoder interface
//  *
//  */
// struct rotary_encoder_t {
//     /**
//      * @brief Filter out glitch from input signals
//      *
//      * @param encoder Rotary encoder handle
//      * @param max_glitch_us Maximum glitch duration, in us
//      * @return
//      *      - ESP_OK: Set glitch filter successfully
//      *      - ESP_FAIL: Set glitch filter failed because of other error
//      */
//     esp_err_t (*set_glitch_filter)(rotary_encoder_t *encoder, uint32_t max_glitch_us);

//     /**
//      * @brief Start rotary encoder
//      *
//      * @param encoder Rotary encoder handle
//      * @return
//      *      - ESP_OK: Start rotary encoder successfully
//      *      - ESP_FAIL: Start rotary encoder failed because of other error
//      */
//     esp_err_t (*start)(rotary_encoder_t *encoder);

//     /**
//      * @brief Stop rotary encoder
//      *
//      * @param encoder Rotary encoder handle
//      * @return
//      *      - ESP_OK: Stop rotary encoder successfully
//      *      - ESP_FAIL: Stop rotary encoder failed because of other error
//      */
//     esp_err_t (*stop)(rotary_encoder_t *encoder);

//     /**
//      * @brief Recycle rotary encoder memory
//      *
//      * @param encoder Rotary encoder handle
//      * @return
//      *      - ESP_OK: Recycle rotary encoder memory successfully
//      *      - ESP_FAIL: rotary encoder memory failed because of other error
//      */
//     esp_err_t (*del)(rotary_encoder_t *encoder);

//     /**
//      * @brief Get rotary encoder counter value
//      *
//      * @param encoder Rotary encoder handle
//      * @return Current counter value (the sign indicates the direction of rotation)
//      */
//     int (*get_counter_value)(rotary_encoder_t *encoder);
//     int32_t encoder_s_pin;
//     int8_t encoder_s_active_low;
//     int16_t last_encoder_count;
//     encoder_fsm_t fsm_state;
//     uint32_t fsm_timer;
//     uint32_t long_pressed_time;
//     uint32_t short_pressed_time;

// };

// /**
//  * @brief Create rotary encoder instance for EC11
//  *
//  * @param config Rotary encoder configuration
//  * @param ret_encoder Returned rotary encoder handle
//  * @return
//  *      - ESP_OK: Create rotary encoder instance successfully
//  *      - ESP_ERR_INVALID_ARG: Create rotary encoder instance failed because of some invalid argument
//  *      - ESP_ERR_NO_MEM: Create rotary encoder instance failed because there's no enough capable memory
//  *      - ESP_FAIL: Create rotary encoder instance failed because of other error
//  */
// esp_err_t rotary_encoder_new_ec11(const rotary_encoder_config_t *config, rotary_encoder_t **ret_encoder);



// /**
//  * @brief Return encoder state (up, down, button pressed)
//  *
//  * @param encoder Returned rotary encoder handle
//  * @return
//  *      - encoder_state_t: TODO
//  */
// encoder_state_t encoder_state(rotary_encoder_t *encoder);

// /**
//  * @brief 
//  *
//  * @param 
//  * @return
//  *      - encoder_state_t: TODO
//  */
// void encoder_command(uint8_t command, uint16_t encoder_commands[ENCODER_SIZE]);


// /**
//  * @brief Return the state of the encoder push button.
//  *
//  * @param 
//  * @return
//  *      State of te button, 0 or 1.
//  */
// uint8_t encoder_push_state(rotary_encoder_t *encoder);

#endif /* KEYS_H_ */
