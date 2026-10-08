#include "memory.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "hardware/regs/addressmap.h"
#include "pico/stdlib.h"
#include "command.h"
#include "device.h"

const uintptr_t SRAM_SIZE = 264*1024;
const uintptr_t ROM_SIZE = 16*1024;

extern char __flash_binary_start;
extern char __flash_binary_end;
extern char __boot2_start__;
extern char __boot2_end__;
extern char __etext;
extern char __data_start__;
extern char __data_end__;
extern char __bss_start__;
extern char __bss_end__;
extern char __HeapLimit;
extern char __StackBottom;
extern char __StackTop;


// Код к заданию 2.1.4
int main(void);

uint32_t data_variable = 100;
uint32_t bss_variable;
uintptr_t code_addr;
uint16_t first;


// Функции 2.1.3

static void row(const char *name, uintptr_t start, uintptr_t end)
{
    printf("%-10s 0x%08x 0x%08x %8u\n", 
    name, (unsigned)start, (unsigned)end, (unsigned)(end-start));
}


void mem_info(void)
{
    printf("%-10s %-10s %-10s %-10s\n", "area", "start", "end", "size");
    row("flash", (uintptr_t)XIP_BASE, (uintptr_t)(XIP_BASE + PICO_FLASH_SIZE_BYTES));
    row("sram", (uintptr_t)SRAM_BASE, (uintptr_t)(SRAM_BASE+ SRAM_SIZE));
    row("rom", (uintptr_t)ROM_BASE, (uintptr_t)(ROM_BASE + ROM_SIZE));

    row("image", (uintptr_t)&__flash_binary_start, (uintptr_t)&__flash_binary_end);
    row("free", (uintptr_t)&__flash_binary_end, (uintptr_t)(XIP_BASE + PICO_FLASH_SIZE_BYTES));
    row("boot2", (uintptr_t)&__boot2_start__, (uintptr_t)&__boot2_end__);
    row("text", (uintptr_t)&__boot2_end__, (uintptr_t)&__etext);

    // data_flash_size = (unsigned)(&__data_end__ - &__data_start__);
    // data_flash_end = (unsigned)&__etext + data_flash_size;
    // printf("Data flash size %8u\n", data_flash_size);
    // printf("Etext: 0x%08x\n", (unsigned)&__etext);
    // printf("Data flash end 0x%08x\n", data_flash_end);

    row("data flash", (uintptr_t)&__etext,  (uintptr_t)(&__etext + (&__data_end__ - &__data_start__)));
    row("data ram", (uintptr_t)&__data_start__, (uintptr_t)&__data_end__);
    row("bss", (uintptr_t)&__bss_start__, (uintptr_t)&__bss_end__);
    row("heap", (uintptr_t)&__bss_end__, (uintptr_t)&__HeapLimit);
    row("stack", (uintptr_t)&__StackBottom, (uintptr_t)&__StackTop);

    printf("\ntotal\n");

    printf("   flash image  %8u = boot2 %u + text %u + data %u\n",

            (unsigned)(&__boot2_end__ - &__boot2_start__) + 
            (unsigned)(&__etext - &__boot2_end__) + 
            (unsigned)(&__data_end__ - &__data_start__),

            (unsigned)(&__boot2_end__ - &__boot2_start__),
            (unsigned)(&__etext - &__boot2_end__),
            (unsigned)(&__data_end__ - &__data_start__));
    
    printf("   flash free   %8u of %u\n",
            (unsigned)(XIP_BASE + PICO_FLASH_SIZE_BYTES) - (unsigned)&__flash_binary_end,
            PICO_FLASH_SIZE_BYTES);

    printf("   ram used     %8u = data %u + bss %u\n",
            (unsigned)(&__data_end__ - &__data_start__) + (unsigned)(&__bss_end__ - &__bss_start__),
            (unsigned)(&__data_end__ - &__data_start__),
            (unsigned)(&__bss_end__ - &__bss_start__));

    printf("   ram free     %8u for hear and %u for stack\n",
            (unsigned)(&__HeapLimit - &__bss_end__),
            (unsigned)(&__StackTop - &__StackBottom));

}

// Функции 2.1.4
void fw_info(void)
{
        data_variable = data_variable+1;
        bss_variable = bss_variable+1;

        printf("%-14s %-10s %-15s\n", "object", "address", "value");

        // Функция main
        code_addr = (uintptr_t)main & ~(uintptr_t)1;
        first = *(uint16_t *)code_addr;
        printf("%-14s 0x%08x 0x%04x\n", "main", code_addr, first);

        // Функция fw_info
        code_addr = (uintptr_t)fw_info & ~(uintptr_t)1;
        first = *(uint16_t *)code_addr;
        printf("%-14s 0x%08x 0x%04x\n", "fw_info", code_addr, first);

        // Адреса обработчиков
        for (uint i=0; i<command_count; i++)
        {
                printf("- %-12s 0x%08x\n", commands[i].name, (uint16_t *)((uintptr_t)commands[i].handler & ~1u));
        }

        //DEVICE INFO
        printf("%-14s 0x%08x %15s\n", "DEVICE_PROJECT", (void *)DEVICE_PROJECT, DEVICE_PROJECT);
        printf("%-14s 0x%08x %15s\n", "DEVICE_BOARD", (void *)DEVICE_BOARD, DEVICE_BOARD);

        // Variables
        printf("%-14s 0x%08x %u\n", "data_variable", (uintptr_t)&data_variable, data_variable);
        printf("%-14s 0x%08x %u\n", "bss_variable", (uintptr_t)&bss_variable, bss_variable);


        // Stack and heap
        uint32_t stack_variable = 1946;
        uint32_t *heap_variable = malloc(sizeof(uint32_t));

        if (heap_variable != NULL)
        {
                *heap_variable = 1951;
        }

        printf("%-14s 0x%08x %u\n", "stack_variable", (uintptr_t)&stack_variable, stack_variable);
        printf("%-14s 0x%08x %u\n", "heap_variable", (uintptr_t)heap_variable, *heap_variable);

        free(heap_variable);

}