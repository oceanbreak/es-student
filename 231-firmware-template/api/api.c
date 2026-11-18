#include "api.h"
#include "pico/stdlib.h"
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include "led-task.h"
#include "log.h"
#include "device.h"
#include "profiling.h"
#include "button-task.h"

#define CALC_PI_TERMS 1000000;
// прикидка: за член ряда 4 операции с double, 175 + 110 + 190 + 110 = 585 тактов;
// 1 000 000 членов по 585 тактов при 125 МГц — около 4,7 с
// const uint32_t CALC_PI_TERMS = 1000000;
volatile double pi_result;

void cmd_help(const command_t *command);

// обработчик команды: получает имя и аргументы
typedef void (*api_callback_t)(const command_t *command);


// команда прибора: имя, справка для help и обработчик
typedef struct
{
    const char *name;
    const char *help;
    api_callback_t callback;
} api_command_t;


// Число без знака из десятичной записи. false — в записи не только цифры
// или число не помещается в uint32_t.
static bool parse_u32(const char *text, uint32_t *value)
{
    // strtoul приняла бы и знак: из "-1" получилось бы 4294967295
    if (!isdigit((unsigned char)text[0]))
    {
        return false;
    }

    char *end;
    errno = 0;
    unsigned long number = strtoul(text, &end, 10);

    // после числа остались символы или число не поместилось
    if (*end != '\0' || errno == ERANGE || number > UINT32_MAX)
    {
        return false;
    }

    *value = (uint32_t)number;
    return true;
}



static double calc_pi(uint terms)
{
    double sum = 0.0;
    double sign = 1.0;

    for (int k = 0; k < terms; k++) {
        // Прибавить очередной член ряда: ±1 / (2k + 1)
        sum += sign / (2.0 * k + 1.0);
        sign = -sign;
    }

    // Сумма ряда равна π/4
    return 4.0 * sum;
}


//---------- ОБРАБОТЧИКИ КОМАНД ----------------

static void cmd_calc_pi(const command_t *command)
{
    uint32_t calc_pi_terms_value = CALC_PI_TERMS;
    if (command->argc > 0)
    {
        parse_u32(command->argv[0], &calc_pi_terms_value);
    }

    uint64_t start_us = time_us_64();
    pi_result = calc_pi(calc_pi_terms_value);
    uint64_t spent_us = time_us_64() - start_us;

    printf("pi: %.8f\n", pi_result);
    printf("time: %llu ms\n", spent_us / 1000);
}


static void cmd_info(const command_t *command)
{
    // печатаем паспорт устройства
    device_info();
}

static void cmd_uptime(const command_t *command)
{
    printf("uptime: %llu ms\n", time_us_64() / 1000);
}

static void main_time_exec(const command_t *command)
{
    printf("iteration avg %.2f us, max %u us\n", profiling_avg_us(), (unsigned)profiling_max_us());
}

static void main_time_reset(const command_t *command)
{
    profiling_reset_max();
}

static void print_led_state(void)
{
    uint32_t period_ms = led_task_get_period_ms();
    led_state_t led_state = led_task_get_state();
    switch (led_state)
    {
        case LED_STATE_ON:
        {
            printf("led: on, period %u ms\n", (unsigned)period_ms);
            break;
        }
        case LED_STATE_OFF:
        {
            printf("led: off, period %u ms\n", (unsigned)period_ms);
            break;
        }
        case LED_STATE_BLINK:
        {
            printf("led: blink, period %u ms\n", (unsigned)period_ms);
            break;
        }
    }
}

static void cmd_led_enable(const command_t *command)
{
    led_task_set_state(LED_STATE_ON);
    print_led_state();
}

static void cmd_led_disable(const command_t *command)
{
    led_task_set_state(LED_STATE_OFF);
    print_led_state();
}

static void cmd_led_blink(const command_t *command)
{
    led_task_set_state(LED_STATE_BLINK);
    print_led_state();
}

static void cmd_led_period(const command_t *command)
{
    uint32_t period_ms;

    if (command->argc != 1 || !parse_u32(command->argv[0], &period_ms))
    {
        printf("error: usage led_period <period_ms>\n");
        return;
    }

    // годится ли такой период, решает задача светодиода
    if (!led_task_set_period_ms(period_ms))
    {
        printf("error: period_ms must be greater than 0\n");
        return;
    }

    // ответ: состояние и период светодиода
    led_task_set_period_ms(period_ms);
    print_led_state();
}

static void cmd_button(const command_t* command)
{
    if (button_task_is_pressed())
    {
        printf("button: pressed, presses %i\n", button_task_get_press_count());
    }
    else
    {
        printf("button: released, presses %i\n", button_task_get_press_count());
    }

}


// -----------------------------------------------------------


// СОбираем обработчики в таблицу
const api_command_t api_commands[] = {
    {"help", "list of commands", cmd_help},
    {"info", "device passport", cmd_info},
    {"uptime", "time since reset", cmd_uptime},
    {"calc_pi", "calculate pi: calc_pi [terms]", cmd_calc_pi},
    {"main_time_exec", "superloop iteration: average and maximum", main_time_exec},
    {"main_time_reset", "reset maximum iteration", main_time_reset},
    {"led_enable", "set led ON", cmd_led_enable},
    {"led_disable", "set led OFF", cmd_led_disable},
    {"led_blink", "set led in blink mode", cmd_led_blink},
    {"led_period", "set blink period", cmd_led_period},
    {"button", "get button state and press count", cmd_button},
};



// Общее количество команд
uint32_t api_command_count = sizeof(api_commands) / sizeof(api_commands[0]);


// Help отдельно
void cmd_help(const command_t *command)
{
    for (uint32_t i=0; i < api_command_count; i++)
    {
        printf("%-20s %s\n", api_commands[i].name, api_commands[i].help);
    }
}


// Проверка команды
void api_handle(const command_t *command)
{
    if (command->truncated)
    {
        printf("error: command longer than %u characters or %u arguments\n", 
            STDIO_TEXT_PROTOCOL_LINE_SIZE,
            STDIO_TEXT_PROTOCOL_MAX_ARGS);
        return;
    }
    for (uint32_t i=0; i<api_command_count; i++)
    {
        if (strcmp(command->name, api_commands[i].name)==0)
        {
            api_commands[i].callback(command);
            return;
        }
    }
    printf("error: unknown command '%s', try help\n", command->name);
}