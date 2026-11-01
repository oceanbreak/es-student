#include "api.h"
#include "pico/stdlib.h"
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include "led.h"
#include "log.h"
#include "device.h"
#include "profiling.h"

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

void cmd_calc_pi(const command_t *command)
{
    uint32_t calc_pi_terms_value;
    if (command->argc > 0)
    {
        parse_u32(command->argv[0], &calc_pi_terms_value);
    }
    else
    {
        calc_pi_terms_value = CALC_PI_TERMS;
    }
    uint64_t start_us = time_us_64();
    pi_result = calc_pi(calc_pi_terms_value);
    uint64_t spent_us = time_us_64() - start_us;

    printf("pi: %.8f\n", pi_result);
    printf("time: %llu ms\n", spent_us / 1000);
}


void cmd_info(const command_t *command)
{
    // печатаем паспорт устройства
    device_info();
}

void cmd_uptime(const command_t *command)
{
    printf("uptime: %llu ms\n", time_us_64() / 1000);
}

void main_time_exec(const command_t *command)
{
    printf("iteration avg %.2f us, max %u us\n", profiling_avg_us(), (unsigned)profiling_max_us());
}

void main_time_reset(const command_t *command)
{
    profiling_reset_max();
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