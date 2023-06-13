/**
 * @file ota.h
 * @author Yeferson Flores
 * @brief Header for ota.c file
 * 
 * @copyright Copyright (c) 2023. Deepsea Developments SAS.
 * 
 */

#ifndef MAIN_OTA_H_
#define MAIN_OTA_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "common.h"

#define STACK_SIZE_OTA_TASK     4096

#define PRIORITY_OTA_TASK        1  

#define OTA_TASK_TAG            "OTA_T"

TaskHandle_t otaTaskHandle;

void ota_init(void);

#endif