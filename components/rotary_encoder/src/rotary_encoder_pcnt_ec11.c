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
#include <stdlib.h>
#include <string.h>
#include <sys/cdefs.h>
#include <inttypes.h>
#include "esp_compiler.h"
#include "esp_log.h"
#include "driver/pcnt.h"
#include "sys/lock.h"
#include "hal/pcnt_hal.h"
#include "rotary_encoder.h"

#include "hal_ble.h"
#include "keypress_handles.h"

static const char *TAG = "rotary_encoder";

#define ROTARY_CHECK(a, msg, tag, ret, ...)                                       \
    do {                                                                          \
        if (unlikely(!(a))) {                                                     \
            ESP_LOGE(TAG, "%s(%d): " msg, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
            ret_code = ret;                                                       \
            goto tag;                                                             \
        }                                                                         \
    } while (0)

#define EC11_PCNT_DEFAULT_HIGH_LIMIT (100)
#define EC11_PCNT_DEFAULT_LOW_LIMIT  (-100)

// A flag to identify if pcnt isr service has been installed.
static bool is_pcnt_isr_service_installed = false;
// A lock to avoid pcnt isr service being installed twice in multiple threads.
static _lock_t isr_service_install_lock;
#define LOCK_ACQUIRE() _lock_acquire(&isr_service_install_lock)
#define LOCK_RELEASE() _lock_release(&isr_service_install_lock)

typedef struct {
    int accumu_count;
    rotary_encoder_t parent;
    pcnt_unit_t pcnt_unit;
} ec11_t;

static esp_err_t ec11_set_glitch_filter(rotary_encoder_t *encoder, uint32_t max_glitch_us)
{
    esp_err_t ret_code = ESP_OK;
    ec11_t *ec11 = __containerof(encoder, ec11_t, parent);

    /* Configure and enable the input filter */
    ROTARY_CHECK(pcnt_set_filter_value(ec11->pcnt_unit, max_glitch_us * 80) == ESP_OK, "set glitch filter failed", err, ESP_FAIL);

    if (max_glitch_us) {
        pcnt_filter_enable(ec11->pcnt_unit);
    } else {
        pcnt_filter_disable(ec11->pcnt_unit);
    }

    return ESP_OK;
err:
    return ret_code;
}

static esp_err_t ec11_start(rotary_encoder_t *encoder)
{
    ec11_t *ec11 = __containerof(encoder, ec11_t, parent);
    pcnt_counter_resume(ec11->pcnt_unit);
    return ESP_OK;
}

static esp_err_t ec11_stop(rotary_encoder_t *encoder)
{
    ec11_t *ec11 = __containerof(encoder, ec11_t, parent);
    pcnt_counter_pause(ec11->pcnt_unit);
    return ESP_OK;
}

static int ec11_get_counter_value(rotary_encoder_t *encoder)
{
    ec11_t *ec11 = __containerof(encoder, ec11_t, parent);
    int16_t val = 0;
    pcnt_get_counter_value(ec11->pcnt_unit, &val);
    return (val + ec11->accumu_count)/2;
}

static esp_err_t ec11_del(rotary_encoder_t *encoder)
{
    ec11_t *ec11 = __containerof(encoder, ec11_t, parent);
    free(ec11);
    return ESP_OK;
}

static void ec11_pcnt_overflow_handler(void *arg)
{
    ec11_t *ec11 = (ec11_t *)arg;
    uint32_t status = 0;
    pcnt_get_event_status(ec11->pcnt_unit, &status);

    if (status & PCNT_EVT_H_LIM) {
        ec11->accumu_count += EC11_PCNT_DEFAULT_HIGH_LIMIT;
    } else if (status & PCNT_EVT_L_LIM) {
        ec11->accumu_count += EC11_PCNT_DEFAULT_LOW_LIMIT;
    }
}

esp_err_t rotary_encoder_new_ec11(const rotary_encoder_config_t *config, rotary_encoder_t **ret_encoder)
{
    esp_err_t ret_code = ESP_OK;
    ec11_t *ec11 = NULL;

    ROTARY_CHECK(config, "configuration can't be null", err, ESP_ERR_INVALID_ARG);
    ROTARY_CHECK(ret_encoder, "can't assign context to null", err, ESP_ERR_INVALID_ARG);

    ec11 = calloc(1, sizeof(ec11_t));
    ROTARY_CHECK(ec11, "allocate context memory failed", err, ESP_ERR_NO_MEM);

    ec11->pcnt_unit = (pcnt_unit_t)(config->dev);

    // Configure channel 0
    pcnt_config_t dev_config = {
        .pulse_gpio_num = config->phase_a_gpio_num,
        .ctrl_gpio_num = config->phase_b_gpio_num,
        .channel = PCNT_CHANNEL_0,
        .unit = ec11->pcnt_unit,
        .pos_mode = PCNT_COUNT_DEC,
        .neg_mode = PCNT_COUNT_INC,
        .lctrl_mode = PCNT_MODE_REVERSE,
        .hctrl_mode = PCNT_MODE_KEEP,
        .counter_h_lim = EC11_PCNT_DEFAULT_HIGH_LIMIT,
        .counter_l_lim = EC11_PCNT_DEFAULT_LOW_LIMIT,
    };
    ROTARY_CHECK(pcnt_unit_config(&dev_config) == ESP_OK, "config pcnt channel 0 failed", err, ESP_FAIL);

    // Configure channel 1
    dev_config.pulse_gpio_num = config->phase_b_gpio_num;
    dev_config.ctrl_gpio_num = config->phase_a_gpio_num;
    dev_config.channel = PCNT_CHANNEL_1;
    dev_config.pos_mode = PCNT_COUNT_INC;
    dev_config.neg_mode = PCNT_COUNT_DEC;
    ROTARY_CHECK(pcnt_unit_config(&dev_config) == ESP_OK, "config pcnt channel 1 failed", err, ESP_FAIL);

    // PCNT pause and reset value
    pcnt_counter_pause(ec11->pcnt_unit);
    pcnt_counter_clear(ec11->pcnt_unit);

    // register interrupt handler in a thread-safe way
    LOCK_ACQUIRE();
    if (!is_pcnt_isr_service_installed) {
        ROTARY_CHECK(pcnt_isr_service_install(0) == ESP_OK, "install isr service failed", err, ESP_FAIL);
        // make sure pcnt isr service won't be installed more than one time
        is_pcnt_isr_service_installed = true;
    }
    LOCK_RELEASE();

    pcnt_isr_handler_add(ec11->pcnt_unit, ec11_pcnt_overflow_handler, ec11);

    pcnt_event_enable(ec11->pcnt_unit, PCNT_EVT_H_LIM);
    pcnt_event_enable(ec11->pcnt_unit, PCNT_EVT_L_LIM);

    ec11->parent.del = ec11_del;
    ec11->parent.start = ec11_start;
    ec11->parent.stop = ec11_stop;
    ec11->parent.set_glitch_filter = ec11_set_glitch_filter;
    ec11->parent.get_counter_value = ec11_get_counter_value;
    ec11->parent.encoder_s_pin = config->button_gpio_num;
    ec11->parent.encoder_s_active_low = config->button_active_low;
    ec11->parent.last_encoder_count = 0;
    ec11->parent.fsm_state = S_B_IDLE;
    ec11->parent.fsm_timer = 0;
    ec11->parent.long_pressed_time =  100000;
    ec11->parent.short_pressed_time = 60000;

    //Configure Encoder button
    if(config->button_gpio_num != GPIO_NUM_NC)
    {

        esp_rom_gpio_pad_select_gpio(config->button_gpio_num);
        gpio_set_direction(config->button_gpio_num, GPIO_MODE_INPUT);
        gpio_set_pull_mode(config->button_gpio_num,GPIO_PULLDOWN_ONLY);
    }

    *ret_encoder = &(ec11->parent);

    return ESP_OK;
err:
    if (ec11) {
        free(ec11);
    }
    return ret_code;
}


uint8_t encoder_push_state(rotary_encoder_t *encoder)
{
    return gpio_get_level(encoder->encoder_s_pin) ^ encoder->encoder_s_active_low;
}

//Check encoder state, currently defined for Vol +/= and mute
encoder_state_t encoder_state(rotary_encoder_t *encoder)
{
    uint8_t EncoderState = 0x00;
    int16_t EncoderCount = ENC_IDLE;
    int16_t PastEncoderCount = encoder->last_encoder_count;
    
    EncoderCount = encoder->get_counter_value(encoder);
    
    if(EncoderCount > PastEncoderCount){
        EncoderState = ENC_UP;
        //ESP_LOGI("Encoder","up");
    }
    if(EncoderCount < PastEncoderCount){
        EncoderState = ENC_DOWN;
        //ESP_LOGI("Encoder","down");
    }
    if (encoder->encoder_s_pin != GPIO_NUM_NC)
    {
        //FSM for the button state
        switch(encoder->fsm_state)
        {
            case S_B_IDLE:
                //Button Pressed
                if(encoder_push_state(encoder) == 1)
                {
                    encoder->fsm_state=S_BUTTON_PRESSED;
                    encoder->fsm_timer=0;
                    //ESP_LOGI("Encoder FSM","IDLE->PRESSED");
                }
            break;
            case S_BUTTON_PRESSED:
                //Button Released
                if(encoder_push_state(encoder) == 0)
                {
                    encoder->fsm_state=S_BUTTON_RELEASED;
                    encoder->fsm_timer=0;
                    //ESP_LOGI("Encoder FSM", "PRESSED->RELEASED");
                }
                else
                {
                    encoder->fsm_timer=encoder->fsm_timer+1;
                    //Long Pressed
                    if( encoder->fsm_timer > encoder->long_pressed_time)
                    {
                        encoder->fsm_state=S_LONG_PRESSED;
                        encoder->fsm_timer=0;
                        EncoderState = ENC_BUT_LONG_PRESS;
                        //ESP_LOGI("Encoder","Long Pressed");
                        //ESP_LOGI("Encoder FSM","PRESSED->LONG_PRESSED");
                    }
                }
                
            break;
            case S_BUTTON_RELEASED:
                //Button Pressed
                if(encoder_push_state(encoder) == 1)
                {
                    encoder->fsm_state=S_DOUBLE_PRESSED;
                    encoder->fsm_timer=0;
                    EncoderState = ENC_BUT_DOUBLE_PRESS;
                    //ESP_LOGI("Encoder","Double Pressed");
                    //ESP_LOGI("Encoder FSM","RELEASE->DOUBLE_PRESS");
                }
                else
                {
                    encoder->fsm_timer=encoder->fsm_timer+1;
                    //Short Pressed
                    if( encoder->fsm_timer > encoder->short_pressed_time)
                    {
                        encoder->fsm_state=S_B_IDLE;
                        encoder->fsm_timer=0;
                        EncoderState = ENC_BUT_SHORT_PRESS;
                        //ESP_LOGI("Encoder","Pressed");
                        //ESP_LOGI("Encoder FSM","RELEASED->IDLE");
                    }
                }
                break;

            case S_LONG_PRESSED:
                //Button Released
                if(encoder_push_state(encoder) == 0)
                {
                    encoder->fsm_state=S_B_IDLE;
                    encoder->fsm_timer=0;
                    //ESP_LOGI("Encoder FSM","LONG_PRESSED->IDLE");
                }
            break;

            case S_DOUBLE_PRESSED:
                //Button Released
                if(encoder_push_state(encoder) == 0)
                {
                    encoder->fsm_state=S_B_IDLE;
                    encoder->fsm_timer=0;
                    //ESP_LOGI("Encoder FSM","DOUBLE_PRESSED->IDLE");
                }
            break;
        }
    }

    encoder->last_encoder_count = EncoderCount;
    return EncoderState;
}

// checking if a modifier key was pressed
uint16_t check_key_modifier(uint16_t key) {

    uint8_t cur_mod = 0;
    // these are the modifier keys
    if ((KC_LCTRL <= key) && (key <= KC_RGUI)) {
        cur_mod = (1 << (key - KC_LCTRL));
        return cur_mod;
    }
    return 0;
}


//How to process encoder activity
void encoder_command(uint8_t command, uint16_t encoder_commands[ENCODER_SIZE]){
    uint8_t type = encoder_commands[0];
    uint16_t action;
    uint8_t media_state[2] = {0,0};
    uint8_t mouse_state[5] = {0,0,0,0,0};
    uint8_t key_state[REPORT_LEN] = {0};
    uint8_t modifier = 0;

    if(command != ENC_IDLE)
    {
        action = encoder_commands[command - 1]; //Have to substract 1 as IDLE is the state 0.

        //Review media actions
        if(action >= KC_MEDIA_NEXT_TRACK && action <= KC_MEDIA_REWIND)
        {
            switch(action)
            {
                case KC_MEDIA_NEXT_TRACK:
                    media_state[1] = 10;
                break;

                case KC_MEDIA_PREV_TRACK:
                    media_state[1] = 11;
                break;

                case KC_MEDIA_STOP:
                    media_state[1] = 12;
                break;

                case KC_MEDIA_PLAY_PAUSE:
                    media_state[1] = 5;
                break;

                case KC_AUDIO_MUTE:
                    media_state[1] = 1;
                break;

                case KC_AUDIO_VOL_UP:
                    SET_BIT(media_state[0],6);
                break;

                case KC_AUDIO_VOL_DOWN:
                    SET_BIT(media_state[0],7);
                break;
            }
            
            xQueueSend(media_q,(void*)&media_state, (TickType_t) 0); // Send Command
            media_state[0] = 0;
            media_state[1] = 0;
            xQueueSend(media_q,(void*)&media_state, (TickType_t) 0); // Reset command
        }
        //Review mouse actions
        else if(action >= KC_MS_UP && action <= KC_MS_ACCEL2)
        {
            switch(action)
            {
                case KC_MS_UP:
                    mouse_state[2] = 15;
                    break;

                case KC_MS_DOWN:
                    mouse_state[2] = -15;
                    break;

                case KC_MS_LEFT:
                    mouse_state[1] = -15;
                    break;

                case KC_MS_RIGHT:
                    mouse_state[1] = 15;
                    break;

                case KC_MS_BTN1:
                    mouse_state[0] = 1;
                    break;

                case KC_MS_BTN2:
                    mouse_state[0] = 2;
                    break;

                case KC_MS_WH_UP:
                    mouse_state[3] = 1;
                    break;
                case KC_MS_WH_DOWN:
                    mouse_state[3] = -1;
                    break;
            }
            xQueueSend(mouse_q,(void*)&mouse_state, (TickType_t) 0);
            mouse_state[0] = 0;
            mouse_state[1] = 0;
            mouse_state[2] = 0;
            mouse_state[3] = 0;
            xQueueSend(mouse_q,(void*)&mouse_state, (TickType_t) 0);
        }
        //Review Macro actions
        else if(action >= MACRO_BASE_VAL && action < MACRO_HOLD_MAX_VAL)
        {
            ESP_LOGI("Encoder","Macro detected %d", action);

            for (uint8_t i = 0; i < MACRO_LEN; i++) 
            {
                uint16_t key = macros[action - MACRO_BASE_VAL][i];
                if (key == KC_NO)
                {
                    break;
                }
                key_state[2+i] = key;
                modifier |= check_key_modifier(key);
           
                //ESP_LOGI("KEY sent", "macroid: %d", key);
            }
            key_state[0] = modifier;
            xQueueSend(keyboard_q,(void*)&key_state, (TickType_t) 0);
            for (uint8_t i = 0; i < MACRO_LEN; i++) {
                uint16_t key = macros[action - MACRO_BASE_VAL][i];
                if (key == KC_NO)
                {
                    break;
                }
                key_state[2+i] = 0;
                modifier &= ~check_key_modifier(key);
            }
            key_state[0] = modifier;
            xQueueSend(keyboard_q,(void*)&key_state, (TickType_t) 0);
        }
        // Review Layer actions
        else if(action >= LAYER_ADJUST_MIN && action <= LAYER_ADJUST_MAX)
        {
            ESP_LOGI("Encoder","Layer action detected");
            layer_adjust(action);
        }
        //Review Key actions
        else
        {
            ESP_LOGI("Encoder","Regular key detected: %d", action);
            key_state[2] = action;
            xQueueSend(keyboard_q,(void*)&key_state, (TickType_t) 0);
            key_state[2] = 0;
            xQueueSend(keyboard_q,(void*)&key_state, (TickType_t) 0);
        }
    } 
    
    vTaskDelay(pdMS_TO_TICKS(5));
}
