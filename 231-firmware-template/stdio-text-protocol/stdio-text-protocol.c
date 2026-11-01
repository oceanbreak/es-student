#include "stdio-text-protocol.h"
#include "pico/stdlib.h"
#include <stdio.h>

static char line[STDIO_TEXT_PROTOCOL_LINE_SIZE];
static uint line_length = 0;
static bool truncated = false;
static command_t output_command;




void stdio_text_protocol_init(void)
{
    stdio_init_all();
}

// Читает один символ, не ожидая его. Когда строка закончена, возвращает
// команду, иначе NULL. Команда действительна до следующего вызова.
const command_t *stdio_text_protocol_handle(void)
{

    // читаем символ без ожидания; нет символа — возвращаем NULL
    int symbol = getchar_timeout_us(0);

    if (symbol == PICO_ERROR_TIMEOUT)
    {
        return NULL;
    }
    // конец строки: пустую строку пропускаем, иначе переводим строку в терминале,
    // делим строку на слова и возвращаем команду
    if (symbol == '\n' || symbol=='\r')
    {
   
        if (line_length > 0)
        {
            line[line_length] = '\0';
            putchar('\n');
            // printf("Line length %u\n", line_length);

            // Пишем структуру
            
            output_command.truncated = truncated;
            output_command.argc=0;
            uint word_count = 1;
            char prev_symbol = '\0';
            for (uint i=0; i<line_length; i++)
            {
                if (line[i] == ' ')
                {
                    line[i] = '\0';
                }
                else
                {
                    if (prev_symbol == '\0')
                    {
                        if (word_count > STDIO_TEXT_PROTOCOL_MAX_ARGS + 1){
                            output_command.truncated = true;
                        }
                        if (word_count == 1)
                        {
                            output_command.name = &line[i];
                            word_count = word_count + 1;
                            // printf("Name: %s\n", &line[i]);
                        }
                        else
                        {
                            output_command.argv[word_count-2] = &line[i];
                            output_command.argc = word_count - 1;
                            // printf("Arg %u: %s\n", word_count-1, &line[i]);
                            word_count = word_count + 1;
                        }
                    }
                }
                prev_symbol = line[i];
                
            }
            
            line_length = 0;
            truncated = false;
            return &output_command;

        }
        return NULL;

    }

    // Backspace (0x08 или 0x7f): убираем последний символ и стираем его на экране "\b \b"
    if (symbol == 0x08 || symbol == 0x7f)
    {
        if (line_length > 0)
        {
            line_length = line_length-1;
            printf("\b \b");
            truncated = false;
        }
        return NULL;

    }

    // печатный символ: кладём в буфер и повторяем в терминал;
    // буфер полон — помечаем строку как переполненную   
    if (line_length + 1 < STDIO_TEXT_PROTOCOL_LINE_SIZE)
    {
        line[line_length] = (char)symbol;
        line_length = line_length + 1;
        putchar(symbol);
    }
    else
    {
        truncated = true;
    }
    return NULL;

}

void print_command(const command_t *command)
{
    if (command!=NULL)
    {
        printf("Name: %s 1st arg: %s, args count %u\n", command->name, command->argv[0], command->argc);
    }
    
}