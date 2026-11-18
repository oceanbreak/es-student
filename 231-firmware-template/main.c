#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include <stdio.h>
#include "led-task.h"
#include "log.h"
#include "device.h"
#include <string.h>
#include "profiling.h"
#include "stdio-text-protocol.h"
#include "api.h"
#include "button-task.h"



int main()
{
    stdio_text_protocol_init();
    led_task_init();
    button_task_init();


    profiling_init();

    while(1)
    {
        profiling_iteration();

        // Мигаем
        led_task_handle();

        // Кнопочка
        button_task_handle();

        // Проверяем команду
        const command_t *command = stdio_text_protocol_handle();
        if (command != NULL)
        {
            api_handle(command);
        }


    }
}