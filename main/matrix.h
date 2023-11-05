/*
 * matrix.h
 *
 *  Created on: 6 Oct 2018
 *      Author: gal
 */

#ifndef MATRIX_H_
#define MATRIX_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include "keyboard_config.h"

// extern uint8_t MATRIX_STATE[MATRIX_ROWS][MATRIX_COLS];
// extern uint8_t SLAVE_MATRIX_STATE[MATRIX_ROWS][MATRIX_COLS];
// extern uint8_t (*matrix_states[])[MATRIX_ROWS][MATRIX_COLS];

/*
 * @brief deinitialize rtc pins
 */
void rtc_matrix_deinit(void);

/*
 * @brief initialize rtc pins
 */
void rtc_matrix_setup(void);

/*
 * @brief initialize matrix
 */
void matrix_setup(void);

// /*
//  * @brief scan matrix
//  */
// void scan_matrix(uint8_t * current_matrix);

#endif /* MATRIX_H_ */
