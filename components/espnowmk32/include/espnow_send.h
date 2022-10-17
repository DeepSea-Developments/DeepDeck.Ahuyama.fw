/*
 * espnow_split.h
 *
 *  Created on: 12 Aug 2018
 *      Author: gal
 */

#ifndef ESPNOW_SEND_H_
#define ESPNOW_SEND_H_

#include <freertos/queue.h>

//Queue for sending report from matrix scan
extern QueueHandle_t espnow_matrix_send_q;
extern QueueHandle_t espnow_encoder_send_q;

/** @Setup the function for sending data via espnow
 * */
void espnow_send(void);


#endif /* ESPNOW_SPLIT_H_ */
