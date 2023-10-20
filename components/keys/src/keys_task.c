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

void key_event_to_queue(keys_event_struct_t event)
{
    xQueueSend(keys_q, &event, 0);

    ESP_LOGI(TAG,"Key: %d",event.key_pos);
    ESP_LOGI(TAG,"Event: %d",event.event);
    ESP_LOGI(TAG,"Time: %d",event.time);
    ESP_LOGI(TAG,"counter: %d",event.counter);
}

void keys_task(void)
{
    keys_config_struct_t keys_config;
    keys_config.mode = RAW_MODE;
    keys_config.interval_time = 150;
    keys_config.long_time = 2000;
    keys_config.options = LONG_TRIGGER_AT_TIMEOUT;
    keys_config.enable_v = ENABLE_V_TAP_DANCE | ENABLE_V_LEADER_KEY;
    
    while(1)
    {
        scan_matrix(keys_config, key_event_to_queue);

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void init_keys_task(void)
{
    // Init the keys queue
    keys_q = xQueueCreate(KEYS_Q_SIZE, sizeof(keys_event_struct_t));
    matrix_setup();
    xTaskCreate(keys_task, "Keys task", MEM_ENCODER_TASK, NULL, PRIOR_ENCODER_TASK, NULL);
	ESP_LOGI("Keys task", "initialized");

}
