#ifndef INC_DEEPDECK_TASK_H
#define INC_DEEPDECK_TASK_H

#define TRUNC_SIZE 20


//Task for continually updating the OLED
// ToDo: Add a better way to handle freertos task without running wild.
void oled_task(void *pvParameters);

void battery_reports(void *pvParameters);

void key_reports(void *pvParameters);

void rgb_leds_task(void *pvParameters);

#endif