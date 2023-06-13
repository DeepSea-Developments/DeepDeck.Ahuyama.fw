/**
 * @file ota.c
 * @author Yeferson Flores
 * @brief Handler of OTA dedicated task
 * 
 * @copyright Copyright (c) 2023. Deepsea Developments SAS.
 * 
 */

#include "../include/ota.h"

/* 
 * Function:        initNVS(void)
 * -------------------------------------------------------------------------
 * Initialize the non volatile storage of the MCU.
 * 
 * 
 * returns:         none
 */

static void initNVS(void){

    esp_err_t err = nvs_flash_init();
    if(err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND){
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

}


static void ota_test(void* pvParameters){ 
    (void)pvParameters;

    initNVS();

    const esp_partition_t *partition = esp_ota_get_running_partition();
    ESP_LOGE(OTA_TASK_TAG, "System partition: %s", partition->label);

    esp_ota_img_states_t ota_state;
	if (esp_ota_get_state_partition(partition, &ota_state) == ESP_OK) {
		if (ota_state == ESP_OTA_IMG_PENDING_VERIFY) {
			esp_ota_mark_app_valid_cancel_rollback();
		}
	}


    for(;;){                                                               

        vTaskDelay(pdMS_TO_TICKS(50));  //50
    }

}

void ota_init(void){

    otaTaskHandle = NULL;

    if(xTaskCreate(ota_test,                    //function pointer that creates the task
                   "OTA task",                  //ASCII Task name, only for human recognition
                   STACK_SIZE_OTA_TASK,         //task size (WORD)
                   (void*)NULL,                 //init parameters
                   PRIORITY_OTA_TASK,           //task priority
                   &otaTaskHandle )!= pdPASS)   //task handler
    {
        for(;;){} //should no enter here
    }
}