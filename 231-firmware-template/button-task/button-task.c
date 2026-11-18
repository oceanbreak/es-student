#include "button-task.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "led-task.h"

const uint BUTTON_PIN=15;
const uint DEBOUNCE_US = 20000;
uint32_t debounce_time_start_us = 0;
uint16_t button_press_count = 0;

typedef enum {
    BUTTON_PUSHED,
    BUTTON_RELEASED,
    BUTTON_IS_BEING_PUSHED,
    BUTTON_IS_BEING_RELEASED,
} button_state_t;

button_state_t button_state = BUTTON_RELEASED;


void button_task_init(void)
{
    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);
}


void button_task_handle(void)
{
    bool level = gpio_get(BUTTON_PIN);
    uint32_t now_us = time_us_64();

    switch (button_state)
    {
    case BUTTON_RELEASED:
        if (level==0)
        {
            debounce_time_start_us = now_us;
            button_state = BUTTON_IS_BEING_PUSHED;
        }
        break;
    
    case BUTTON_IS_BEING_PUSHED:
    {
        if ((now_us - debounce_time_start_us) > DEBOUNCE_US)
        {
            if (level==1)
            {
                button_state = BUTTON_RELEASED;
                break;
            }
            button_state = BUTTON_PUSHED;
            button_press_count += 1;
            led_task_next_state();
        }
        break;

    }
    case BUTTON_PUSHED:
    {
        if (level==0)
        {
            debounce_time_start_us = now_us;
            button_state = BUTTON_IS_BEING_RELEASED;
        }
        break;
    }
    case BUTTON_IS_BEING_RELEASED:
    {
        if ((now_us - debounce_time_start_us) > DEBOUNCE_US)
        {
            if (level==0)
            {
                button_state = BUTTON_PUSHED;
                break;
            }
            button_state = BUTTON_RELEASED;
        }
        break;
    }
    }
}

bool button_task_is_pressed(void)
{
    if (button_state == (BUTTON_PUSHED | BUTTON_IS_BEING_RELEASED))
    {
        return true;
    }
    return false;
}

uint16_t button_task_get_press_count(void)
{
    return button_press_count;
}