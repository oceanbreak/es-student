#pragma once

#include <stdint.h>


extern uint16_t number_of_presses;

void button_task_init(void);
void button_task_handle(void);

bool button_task_is_pressed(void);
uint16_t button_task_get_press_count(void);