#include "led-task.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include <stdint.h>



static const uint LED_PIN=25;
static bool led_on  = false;
static uint32_t half_period_us = 500000;
led_state_t led_state;
static uint64_t last_toggle_us = 0;

// Проверка времени мигания
static bool period_elapsed(uint64_t *last_us, uint32_t period_us)
{
    uint32_t now_us = time_us_64();
    
    if (now_us - *last_us < period_us)
    {
        return false;
    }

    *last_us += period_us;

    if (now_us - *last_us >= period_us)
    {
        *last_us = now_us;
    }
    return true;
}


static void led_set(bool on)
{
    led_on = on;
    gpio_put(LED_PIN, on);
}

static void led_toggle(void)
{
    led_set(!led_on);
}


void led_task_init(void)
{
    // настраиваем вывод светодиода, период 1000 мс, состояние «мигает»
    led_state = LED_STATE_BLINK;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
}

void led_task_set_state(led_state_t state)
{
    // запоминаем состояние и выполняем действие при входе:
    // «выключен» — гасим, «горит» — зажигаем, «мигает» — первое переключение сразу
    led_state = state;
    switch (state)
    {
    case LED_STATE_ON:
        led_set(true);
        break;

    case LED_STATE_OFF:
        led_set(false);
        break;
    
    case LED_STATE_BLINK:
        led_toggle();
        last_toggle_us = time_us_64();
        break;
    }
}

led_state_t led_task_get_state(void)
{
    return led_state;
}

void led_task_handle(void)
{
    switch (led_state)
    {
        // «выключен» и «горит» — ничего не делаем, светодиод установлен при входе
        case LED_STATE_OFF:
        {break;}

        case LED_STATE_ON:
        {break;}
        
        // «мигает» — каждые полпериода переключаем светодиод
        case LED_STATE_BLINK:
        {
            if (period_elapsed(&last_toggle_us, half_period_us))
            {
                led_toggle();
            }
        }
    }
}

// Установка периода мигания
bool led_task_set_period_ms(uint32_t period_ms)
{
    if (period_ms <= 0)
    {
        return false;
    }
    else
    
    half_period_us = period_ms * 500;
    return true;
}

uint32_t led_task_get_period_ms(void)
{
    return (half_period_us * 2) / 1000;
}

//Переключение состояния в следующее
void led_task_next_state(void)
{
    led_state = (led_state + 1) % LED_STATE_NUM;
    led_task_set_state(led_state);
}
