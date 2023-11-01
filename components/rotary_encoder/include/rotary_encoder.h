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

#ifndef ROTARY_ENCODER_H
#define ROTARY_ENCODER_H

#include "esp_err.h"
#include "key_definitions.h"
#include "keyboard_config.h"

/**
 * @brief Type of Rotary underlying device handle
 *
 */
typedef void *rotary_encoder_dev_t;

/** 
 * @brief State of the rotary encoder 
 * 
 */
typedef enum {
    /** Press event issued */
    ENC_IDLE = 0,
    ENC_UP,
    ENC_DOWN,
    ENC_BUT_SHORT_PRESS,
    ENC_BUT_LONG_PRESS,
    ENC_BUT_DOUBLE_PRESS, 
} encoder_state_t;

/** 
 * @brief Finit state machine states for beign able to do 
 * 
 */
typedef enum {
    S_B_IDLE = 0,
    S_BUTTON_PRESSED,
    S_BUTTON_RELEASED,
    S_LONG_PRESSED,
    S_DOUBLE_PRESSED
 
} encoder_fsm_t;

/**
 * @brief Type of rotary encoder configuration
 *
 */
typedef struct {
    rotary_encoder_dev_t dev; /*!< Underlying device handle */
    int phase_a_gpio_num;     /*!< Phase A GPIO number */
    int phase_b_gpio_num;     /*!< Phase B GPIO number */
    int button_gpio_num;      /*!< Button GPIO number  */
    int button_active_low;    /*!< Button ActiveLow=1 ActiveHigh=0=default */
    int flags;                /*!< Extra flags */
} rotary_encoder_config_t;

/**
 * @brief Default rotary encoder configuration
 *
 */
#define ROTARY_ENCODER_DEFAULT_CONFIG(dev_hdl, gpio_a, gpio_b, gpio_pin, button_al) \
    {                                                                               \
        .dev = dev_hdl,                                                             \
        .phase_a_gpio_num = gpio_a,                                                 \
        .phase_b_gpio_num = gpio_b,                                                 \
        .button_gpio_num = gpio_pin,                                                \
        .button_active_low = button_al,                                             \
        .flags = 0,                                                                 \
    }

/**
 * @brief Type of rotary encoder handle
 *
 */
typedef struct rotary_encoder_t rotary_encoder_t;

/**
 * @brief Rotary encoder interface
 *
 */
struct rotary_encoder_t {
    /**
     * @brief Filter out glitch from input signals
     *
     * @param encoder Rotary encoder handle
     * @param max_glitch_us Maximum glitch duration, in us
     * @return
     *      - ESP_OK: Set glitch filter successfully
     *      - ESP_FAIL: Set glitch filter failed because of other error
     */
    esp_err_t (*set_glitch_filter)(rotary_encoder_t *encoder, uint32_t max_glitch_us);

    /**
     * @brief Start rotary encoder
     *
     * @param encoder Rotary encoder handle
     * @return
     *      - ESP_OK: Start rotary encoder successfully
     *      - ESP_FAIL: Start rotary encoder failed because of other error
     */
    esp_err_t (*start)(rotary_encoder_t *encoder);

    /**
     * @brief Stop rotary encoder
     *
     * @param encoder Rotary encoder handle
     * @return
     *      - ESP_OK: Stop rotary encoder successfully
     *      - ESP_FAIL: Stop rotary encoder failed because of other error
     */
    esp_err_t (*stop)(rotary_encoder_t *encoder);

    /**
     * @brief Recycle rotary encoder memory
     *
     * @param encoder Rotary encoder handle
     * @return
     *      - ESP_OK: Recycle rotary encoder memory successfully
     *      - ESP_FAIL: rotary encoder memory failed because of other error
     */
    esp_err_t (*del)(rotary_encoder_t *encoder);

    /**
     * @brief Get rotary encoder counter value
     *
     * @param encoder Rotary encoder handle
     * @return Current counter value (the sign indicates the direction of rotation)
     */
    int (*get_counter_value)(rotary_encoder_t *encoder);
    int32_t encoder_s_pin;
    int8_t encoder_s_active_low;
    int16_t last_encoder_count;
    encoder_fsm_t fsm_state;
    uint32_t fsm_timer;
    uint32_t long_pressed_time;
    uint32_t short_pressed_time;

};

/**
 * @brief Create rotary encoder instance for EC11
 *
 * @param config Rotary encoder configuration
 * @param ret_encoder Returned rotary encoder handle
 * @return
 *      - ESP_OK: Create rotary encoder instance successfully
 *      - ESP_ERR_INVALID_ARG: Create rotary encoder instance failed because of some invalid argument
 *      - ESP_ERR_NO_MEM: Create rotary encoder instance failed because there's no enough capable memory
 *      - ESP_FAIL: Create rotary encoder instance failed because of other error
 */
esp_err_t rotary_encoder_new_ec11(const rotary_encoder_config_t *config, rotary_encoder_t **ret_encoder);



/**
 * @brief Return encoder state (up, down, button pressed)
 *
 * @param encoder Returned rotary encoder handle
 * @return
 *      - encoder_state_t: TODO
 */
encoder_state_t encoder_state(rotary_encoder_t *encoder);

/**
 * @brief 
 *
 * @param 
 * @return
 *      - encoder_state_t: TODO
 */
void encoder_command(uint8_t command, uint16_t encoder_commands[ENCODER_SIZE]);


/**
 * @brief Return the state of the encoder push button.
 *
 * @param 
 * @return
 *      State of te button, 0 or 1.
 */
uint8_t encoder_push_state(rotary_encoder_t *encoder);

#endif /* ROTARY_ENCODER_H_ */
