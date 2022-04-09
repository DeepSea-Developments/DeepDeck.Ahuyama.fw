/*
 *      ping.h
 *
 *      Created on: Oct 20, 2021
 *      Author: David Alejandro Vallejo
 */
#ifndef MAIN_PING_H_
#define MAIN_PING_H_

#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "ping/ping_sock.h"
#include "esp_log.h"

#define TAG_PING "PING"

typedef struct {
    uint32_t loss;
    uint32_t total_time_ms;
	uint32_t avg_time_ms;
} PING_RESULTS;

extern PING_RESULTS	ping_results;

int do_ping();

#endif /* MAIN_PING_H_ */