/**
 * @file webApp.h
 * @author FelipeMejia (dmejia@dsd.dev)
 * @brief 
 * @version 0.1
 * @date 2023-09-01
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef WEBAPPS_H_
#define WEBAPPS_H_

#include <string.h>
#include <stdlib.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include "keyboard_config.h"
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_netif.h>
#include <cJSON.h>

/**
 * @brief URL of google script
 * 
 * The script made with google Script platform must be implemented like public Web app with all the permisions allowed, but
 * with execution like "User that access to web app" option enabled, this allow to use the script with the credentials of
 * the person that will use the API and instead a common account. At the moment were the script will be deployed, the 
 * system makes an unique ID that must typed here
 */
#define UNIQUE_ID   "AKfycbyWwHvCBtVd_IYjnLHaY_ldWVXf2gvOwCCd5F2WBJjy_SS7eVmYNfgRcHQW-IIkN5lJ"
#define GCALURL     "https://script.google.com/macros/s/" UNIQUE_ID "/exec"

#define TIMEZONE	-5          // Eastern Standard Time (USA)

/**
 * @brief NTP definitions
 * 
 * NTP Server, port and sync period 
 */
#define NTP_SERVER	10,0,0,12
#define NTP_PORT	123
#define NTP_SYNC	60

#define TASK_WEBAPPS_TAG                    "WEB APPS TASK"

void webApps(void *params);

#endif