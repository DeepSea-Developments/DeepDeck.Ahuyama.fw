/**
 * @file keys.c
 * @author ElectroNick (nick@dsd.dev)
 * @brief Component Freertos integration of the keys component.
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

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "keyboard_config.h"
#include "esp_log.h"
#include "esp_timer.h"


static const char *TAG = "keys_tasks";
QueueHandle_t keys_q;
QueueHandle_t keys_config_q;

void key_event_to_queue(keys_event_struct_t event)
{
    xQueueSend(keys_q, &event, 0);

    ESP_LOGI(TAG,"Key: %d",event.key_pos);
    ESP_LOGW(TAG,"Event: %d",event.event);
    ESP_LOGI(TAG,"Time: %d",event.time);
    ESP_LOGI(TAG,"counter: %d",event.counter);
}

void keys_task(void *pvParameters)
{
    keys_config_struct_t keys_config = {
        .mode_vector = {0},
        .general_mode = KEY_CONFIG_NORMAL_MODE,
        .interval_time = 150,
        .long_time = 500
    };

    // keys_config.mode_vector[12] = MODE_V_TAPDANCE_ENABLE;
    // keys_config.mode_vector[13] = MODE_V_TAPDANCE_ENABLE;
    // keys_config.mode_vector[14] = MODE_V_TAPDANCE_ENABLE;
    
    while(1)
    {
        //Check if there has been a change on the key configuration.
        if(xQueueReceive(keys_config_q,&keys_config,0))
        {  
            // TODO: Here i might only change certain parameters i'm interested in, not everything.
            
            
            ESP_LOGI(TAG, "Configuration Received!!");
        }
        scan_matrix(keys_config, key_event_to_queue);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void init_keys_task(void)
{
    // Init the keys queue
    keys_q = xQueueCreate(KEYS_Q_SIZE, sizeof(keys_config_struct_t));

    keys_config_q = xQueueCreate(KEYS_CONFIG_Q_SIZE, sizeof(keys_event_struct_t));
    matrix_setup();
    xTaskCreate(keys_task, "Keys task", MEM_ENCODER_TASK, NULL, PRIOR_ENCODER_TASK, NULL);
	ESP_LOGI("Keys task", "initialized");

}
