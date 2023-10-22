/**
 * @file keys.c
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

#include <stdlib.h>

#include "keys.h"


#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "keyboard_config.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "esp_timer.h"


static const char *TAG = "keys";
/* Define pins, notice that:
 * GPIO6-11 are usually used for SPI flash
 * GPIO34-39 can only be set as input mode and do not have software pullup or pulldown functions.
 * GPIOS 0,2,4,12-15,25-27,32-39 Can be used as RTC GPIOS as well (please read about power management in ReadMe)
 */
const gpio_num_t MATRIX_ROWS_PINS[] = { GPIO_NUM_0, GPIO_NUM_4, GPIO_NUM_5,
		GPIO_NUM_12 };
//const gpio_num_t MATRIX_COLS_PINS[] = { GPIO_NUM_13, GPIO_NUM_14, GPIO_NUM_15, GPIO_NUM_16 };
const gpio_num_t MATRIX_COLS_PINS[] = { GPIO_NUM_16, GPIO_NUM_15, GPIO_NUM_14, 
		GPIO_NUM_13 };


/** 
 * @brief Finite state machine states for key hevaviour
 * 
 */
typedef enum {
    S_IDLE = 0,
    S_PRESSED,
    S_RELEASED,
    S_LONG_P,
    S_TAPDANCE
} keys_basic_fsm_t;


// state matrix aux definitions
#define STATE_MATRIX_STATE_MASK                 0b00011111
#define STATE_MATRIX_DEBOUNCING_FLAG            (1 << 5)
#define STATE_MATRIX_CURRENT_PIN_LEVEL_MASK     (1 << 6)
#define STATE_MATRIX_PAST_PIN_LEVEL_MASK        (1 << 7)

#define STATE_MATRIX_GET_PAST_LVL(x)            ((x & STATE_MATRIX_PAST_PIN_LEVEL_MASK) && STATE_MATRIX_PAST_PIN_LEVEL_MASK)
#define STATE_MATRIX_SET_PAST_LVL(x)            (x | STATE_MATRIX_PAST_PIN_LEVEL_MASK)
#define STATE_MATRIX_RESET_PAST_LVL(x)          (x & ~STATE_MATRIX_PAST_PIN_LEVEL_MASK)

#define STATE_MATRIX_GET_FMS_STATE(x)           (x & STATE_MATRIX_STATE_MASK)

#define STATE_MATRIX_SET_DEBOUNCING(x)          (x | STATE_MATRIX_DEBOUNCING_FLAG)
#define STATE_MATRIX_RESET_DEBOUNCING(x)        (x & ~STATE_MATRIX_DEBOUNCING_FLAG)
#define STATE_MATRIX_IS_DEBOUNCING(x)           (x | STATE_MATRIX_DEBOUNCING_FLAG)

uint32_t lastDebounceTime = 0;

//used for debouncing
static uint32_t millis() {
	return esp_timer_get_time() / 1000;
}

// deinitializing rtc matrix pins on  deep sleep wake up
void rtc_matrix_deinit(void) {

	// Deinitializing columns
	for (uint8_t col = 0; col < MATRIX_COLS; col++) {

		if (rtc_gpio_is_valid_gpio(MATRIX_COLS_PINS[col]) == 1) {
			rtc_gpio_set_level(MATRIX_COLS_PINS[col], 0);
			rtc_gpio_set_direction(MATRIX_COLS_PINS[col],
					RTC_GPIO_MODE_DISABLED);
			gpio_reset_pin(MATRIX_COLS_PINS[col]);
		}
	}

	// Deinitializing rows
	for (uint8_t row = 0; row < MATRIX_ROWS; row++) {

		if (rtc_gpio_is_valid_gpio(MATRIX_ROWS_PINS[row]) == 1) {
			rtc_gpio_set_level(MATRIX_ROWS_PINS[row], 0);
			rtc_gpio_set_direction(MATRIX_ROWS_PINS[row],
					RTC_GPIO_MODE_DISABLED);
			gpio_reset_pin(MATRIX_ROWS_PINS[row]);
		}
	}
}

// Initializing rtc matrix pins for deep sleep wake up
void rtc_matrix_setup(void) {
	uint64_t rtc_mask = 0;

	// Initializing columns
	for (uint8_t col = 0; col < MATRIX_COLS; col++) {

		if (rtc_gpio_is_valid_gpio(MATRIX_COLS_PINS[col]) == 1) {
			rtc_gpio_init((MATRIX_COLS_PINS[col]));
			rtc_gpio_set_direction(MATRIX_COLS_PINS[col],
					RTC_GPIO_MODE_INPUT_OUTPUT);
			rtc_gpio_set_level(MATRIX_COLS_PINS[col], 1);

			ESP_LOGI(TAG,"%d is level %d", MATRIX_COLS_PINS[col],
					gpio_get_level(MATRIX_COLS_PINS[col]));
		}
	}

	// Initializing rows
	for (uint8_t row = 0; row < MATRIX_ROWS; row++) {

		if (rtc_gpio_is_valid_gpio(MATRIX_ROWS_PINS[row]) == 1) {
			rtc_gpio_init((MATRIX_ROWS_PINS[row]));
			rtc_gpio_set_direction(MATRIX_ROWS_PINS[row],
					RTC_GPIO_MODE_INPUT_OUTPUT);
			rtc_gpio_set_drive_capability(MATRIX_ROWS_PINS[row],
					GPIO_DRIVE_CAP_0);
			rtc_gpio_set_level(MATRIX_ROWS_PINS[row], 0);
			rtc_gpio_wakeup_enable(MATRIX_ROWS_PINS[row], GPIO_INTR_HIGH_LEVEL);
			SET_BIT(rtc_mask, MATRIX_ROWS_PINS[row]);

			ESP_LOGI(TAG,"%d is level %d", MATRIX_ROWS_PINS[row],
					gpio_get_level(MATRIX_ROWS_PINS[row]));
		}
		esp_sleep_enable_ext1_wakeup(rtc_mask, ESP_EXT1_WAKEUP_ANY_HIGH);
	}
}

void init_event_struct(keys_event_struct_t * event, key_events_t k_event, uint8_t key_pos, uint32_t time, uint8_t counter)
{  
    event->event = k_event;
    event->key_pos = key_pos;
    event->time = time;
    event->counter = counter ;
}

// Initializing matrix pins
void matrix_setup(void) 
{

#ifdef COL2ROW
	// Initializing columns
	for (uint8_t col = 0; col < MATRIX_COLS; col++) {

		esp_rom_gpio_pad_select_gpio(MATRIX_COLS_PINS[col]);
		gpio_set_direction(MATRIX_COLS_PINS[col], GPIO_MODE_INPUT_OUTPUT);
		gpio_set_level(MATRIX_COLS_PINS[col], 0);

		ESP_LOGI(TAG,"%d is level %d", MATRIX_COLS_PINS[col],
				gpio_get_level(MATRIX_COLS_PINS[col]));
	}

	// Initializing rows
	for (uint8_t row = 0; row < MATRIX_ROWS; row++) {

		esp_rom_gpio_pad_select_gpio(MATRIX_ROWS_PINS[row]);
		gpio_set_direction(MATRIX_ROWS_PINS[row], GPIO_MODE_INPUT_OUTPUT);
		gpio_set_drive_capability(MATRIX_ROWS_PINS[row], GPIO_DRIVE_CAP_0);
		gpio_set_level(MATRIX_ROWS_PINS[row], 0);

		ESP_LOGI(TAG,"%d is level %d", MATRIX_ROWS_PINS[row],
				gpio_get_level(MATRIX_ROWS_PINS[row]));
	}
#endif
#ifdef ROW2COL
	// Initializing rows
	for(uint8_t row=0; row < MATRIX_ROWS; row++) {

		esp_rom_gpio_pad_select_gpio(MATRIX_ROWS_PINS[row]);
		gpio_set_direction(MATRIX_ROWS_PINS[row], GPIO_MODE_INPUT_OUTPUT);
		gpio_set_level(MATRIX_ROWS_PINS[row], 0);
		ESP_LOGI(TAG,"%d is level %d",MATRIX_ROWS_PINS[row],gpio_get_level(MATRIX_ROWS_PINS[row]));
	}

	// Initializing columns
	for(uint8_t col=0; col < MATRIX_ROWS; col++) {

		esp_rom_gpio_pad_select_gpio(MATRIX_COLS_PINS[col]);
		gpio_set_direction(MATRIX_COLS_PINS[col], GPIO_MODE_INPUT_OUTPUT);
		gpio_set_drive_capability(MATRIX_COLS_PINS[col],GPIO_DRIVE_CAP_0);
		gpio_set_level(MATRIX_COLS_PINS[col], 0);

		ESP_LOGI(TAG,"%d is level %d",MATRIX_COLS_PINS[col],gpio_get_level(MATRIX_COLS_PINS[col]));
	}
#endif
}


// Scanning the matrix for input
void scan_matrix(keys_config_struct_t keys_config, KeyEventHandler handler)
{
	uint8_t current_key_state = 0;
    keys_basic_fsm_t current_fsm_state = S_IDLE;
    uint8_t state_item;
	
    static uint32_t aux_time_matrix[MATRIX_ROWS][MATRIX_COLS] = { 0 };
    static uint32_t time_matrix[MATRIX_ROWS][MATRIX_COLS] = { 0 };
    static uint8_t state_matrix[MATRIX_ROWS][MATRIX_COLS] = { 0 };
        // This state matrix works for knowing the current FSM state, the current pin state and 
        // the previous pin state. To do so, this is the usage of every byte of the array:
        // - bit 0 to bit 4: current state one of (keys_basic_fsm_t)
        // - bit 5: Debouncing flag.
        // - bit 6: Current pin state (maybe not needed)
        // - bit 7: Past pin state

    static uint8_t n_tap=0; //This could/should be a matrix to allow multiple tapdaces at the same time.

#ifdef COL2ROW

    // Setting column pin as low, and checking if the input of a row pin changes.
    for (uint8_t col = 0; col < MATRIX_COLS; col++) 
    {	
        gpio_set_level(MATRIX_COLS_PINS[col], 1);
        for (uint8_t row = 0; row < MATRIX_ROWS; row++) 
        {
            current_key_state = gpio_get_level(MATRIX_ROWS_PINS[row]);

            state_item = state_matrix[row][col];


            // -------- Debounce Routine -------------
            if (STATE_MATRIX_GET_PAST_LVL(state_item) != current_key_state) 
            {
                state_item = STATE_MATRIX_SET_DEBOUNCING(state_item);
                aux_time_matrix[row][col] = millis();
                // PREV_MATRIX_STATE[row][col] = current_key_state;
                if(current_key_state)
                {
                    state_item = STATE_MATRIX_SET_PAST_LVL(state_item);
                }
                else
                {
                    state_item = STATE_MATRIX_RESET_PAST_LVL(state_item);
                }
                state_matrix[row][col] = state_item;
                continue; //As it is debouncing, go to the next key
            }
            
            if (STATE_MATRIX_IS_DEBOUNCING(state_item) && ((millis() - aux_time_matrix[row][col]) > DEBOUNCE)) 
            {
                // ESP_LOGI(TAG,"DEBOUNCED");
                // Debounced value
                // MATRIX_STATE[row][col] = current_key_state; //TBC
                state_item = STATE_MATRIX_RESET_DEBOUNCING(state_item);
            }

            // Debounce ready, lets see the state of the key
            

    uint16_t interval_timeout=keys_config.interval_time; // temp variable

            keys_event_struct_t event;
            uint8_t key_vector_pos = ((row) * MATRIX_COLS) + (col); // turns the matrix position (row and col) to a single vector position
            uint8_t key_mode = keys_config.mode_vector[key_vector_pos];

            current_fsm_state = STATE_MATRIX_GET_FMS_STATE(state_item);
            switch (current_fsm_state)
            {
                case S_IDLE:

                    // -------Key Pressed------
                    if(current_key_state == 1)
                    {
                        current_fsm_state = S_PRESSED;
                        ESP_LOGI(TAG,"KEY%d%d pressed",row + 1,col + 1);
                        time_matrix[row][col] = millis();     //This to know the time the key has been pressed
                        aux_time_matrix[row][col] = millis(); //This to review timeouts

                        if(!MODE_V_IS_RAW_DISABLED(key_mode) && !MODE_V_IS_TAPDANCE_ENABLED(key_mode) && !MODE_V_IS_MODTAP_ENABLED(key_mode))
                        {
                            
                            if (handler) 
                            {
                                init_event_struct(&event, KEY_PRESSED, key_vector_pos,
                                                millis()-time_matrix[row][col],n_tap);
                                handler(event);
                            }
                            ESP_LOGI(TAG,"KEY%d%d KEY_PRESSED",row + 1,col + 1);
                        }
                    }

                    break;
                case S_PRESSED:
                    
                    // -------Key Released with tapdance or modtap enabled------
                    if(current_key_state == 0 && (MODE_V_IS_TAPDANCE_ENABLED(key_mode) || MODE_V_IS_MODTAP_ENABLED(key_mode) ))
                    {
                        current_fsm_state = S_RELEASED;
                        aux_time_matrix[row][col] = millis();
                        n_tap=1;
                        ESP_LOGI(TAG,"KEY%d%d released",row + 1,col + 1);
                    }
                    // -------Key Released without any modes enabled------
                    else if(current_key_state == 0)
                    {
                        current_fsm_state = S_IDLE;
                        if (handler) 
                        {
                            init_event_struct(&event, KEY_RELEASED, key_vector_pos,
                                            millis()-time_matrix[row][col],n_tap);
                            handler(event);
                        }
                        ESP_LOGI(TAG,"KEY%d%d KEY_RELEASED",row + 1,col + 1);
                    }

                    // -------Timeout finished, long pressed if modtap is enabled----------------
                    else if(MODE_V_IS_MODTAP_ENABLED(key_mode) && (millis() - aux_time_matrix[row][col]) > keys_config.long_time)
                    {
                        current_fsm_state = S_LONG_P;
                        // IF simple long press is NOT enabled, send the KEY_LONG_P_TIMEOUT event
                        if(!MODE_V_IS_LONG_P_SIMPLE_ENABLED(key_mode))
                        {
                            if (handler) 
                            {
                                init_event_struct(&event, KEY_MT_LONG_TIMEOUT, key_vector_pos,
                                                millis()-time_matrix[row][col],n_tap);
                                handler(event);
                            }
                            ESP_LOGI(TAG,"KEY%d%d event: KEY_MT_LONG_TIMEOUT",row + 1,col + 1);
                        }
                    }
                
                    break;
                case S_RELEASED:

                    //-------- interval timeout finished --------
                    if((millis() - aux_time_matrix[row][col]) > interval_timeout)
                    {
                        current_fsm_state = S_IDLE;
                        // ModTap enaled
                        if (MODE_V_IS_MODTAP_ENABLED(key_mode))
                        {
                            if (handler) 
                            {
                                init_event_struct(&event, KEY_MT_SHORT, key_vector_pos,
                                                millis()-time_matrix[row][col],n_tap);
                                handler(event);
                            }
                            ESP_LOGI(TAG,"KEY%d%d KEY_MT_SHORT",row + 1,col + 1);
                        }
                        // Tap dance event
                        else
                        {
                            if (handler) 
                            { 
                                init_event_struct(&event, KEY_TAP_DANCE, key_vector_pos,
                                                millis()-time_matrix[row][col],n_tap);
                                handler(event);
                            }
                            ESP_LOGI(TAG,"KEY%d%d KEY_TAP_DANCE with %d taps",row + 1,col + 1,n_tap);
                        }
                    }

                    // -------Key Pressed------
                    if(current_key_state == 1)
                    {
                        current_fsm_state = S_TAPDANCE;
                        ESP_LOGI(TAG,"KEY%d%d pressed",row + 1,col + 1);
                    }
                
                    break;
                case S_LONG_P:

                    // -------Key Released ------
                    if(current_key_state == 0)
                    {
                        current_fsm_state = S_IDLE;
                        if (handler) 
                        {
                            
                                init_event_struct(&event, KEY_MT_LONG, key_vector_pos,
                                                millis()-time_matrix[row][col],n_tap);
                                handler(event);
                            ESP_LOGI(TAG,"KEY%d%d KEY_MT_LONG",row + 1,col + 1);
                        }
                    }
                
                break;
                case S_TAPDANCE:
                    
                    // -------Key Released------
                    if(current_key_state == 0)
                    {
                        current_fsm_state = S_RELEASED;
                        n_tap++;
                        aux_time_matrix[row][col] = millis();
                        ESP_LOGI(TAG,"KEY%d%d TapDance",row + 1,col + 1);
                    }
                
                break;    
                
                default:
                    break;
            }

            //save the item in the matrix once it was used.
            
            state_item &= ~STATE_MATRIX_STATE_MASK;
            state_item |= current_fsm_state;
            
            state_matrix[row][col] = state_item;
        }
        gpio_set_level(MATRIX_COLS_PINS[col], 0);
        
    
#endif
#ifdef ROW2COL
        // Setting row pin as low, and checking if the input of a column pin changes.
        for(uint8_t row=0; row < MATRIX_ROWS; row++) {
            gpio_set_level(MATRIX_ROWS_PINS[row], 0);

            for(uint8_t col=0; col <MATRIX_COLS; col++) {

                current_key_state = gpio_get_level(MATRIX_ROWS_PINS[row]);
                if( PREV_MATRIX_STATE[row][col] != current_key_state) {
                    aux_time_matrix[row][col] = millis();
                }
                PREV_MATRIX_STATE[row][col] = current_key_state;
                if( (millis() - aux_time_matrix[row][col]) > DEBOUNCE) {

                    if( MATRIX_STATE[row][col] != current_key_state) {
                        MATRIX_STATE[row][col] = current_key_state;
                    }

                }
            }
            gpio_set_level(MATRIX_ROWS_PINS[row], 0);
        }
#endif
    }

}
