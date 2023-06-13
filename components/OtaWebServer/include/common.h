/* ==============================================================
 * Programmed by: Diego Felipe Mejia Ruiz
 *                Sergio Castañeda
 * Deepsea Developments SAS.
 * Cali, Colombia
 * ==============================================================
;Firmware server header file
===============================================================*/
#ifndef COMMON_H
#define COMMON_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include <esp_ota_ops.h>
#include "esp_timer.h"
#include "driver/i2c.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include <esp_http_server.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h> 
#include <sys/param.h>
#include "mdns.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include <unistd.h>

#endif