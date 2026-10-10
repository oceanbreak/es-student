#include "device.h"

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "pico/unique_id.h"
#include "hardware/regs/addressmap.h"
#include "hardware/regs/sysinfo.h"
#include "pico/version.h"

// struct info_t device_card;

void device_info(void)
{
    char board_id[PICO_UNIQUE_BOARD_ID_SIZE_BYTES * 2 + 1];
    pico_get_unique_board_id_string(board_id, sizeof(board_id));

    volatile  uint32_t *chip_id = (uint32_t *)(SYSINFO_BASE + SYSINFO_CHIP_ID_OFFSET);
    uint32_t id = *chip_id;
    uint32_t manufacturer = (id & SYSINFO_CHIP_ID_MANUFACTURER_BITS) >> SYSINFO_CHIP_ID_MANUFACTURER_LSB;
    uint32_t part = (id & SYSINFO_CHIP_ID_PART_BITS) >> SYSINFO_CHIP_ID_PART_LSB;
    uint32_t revision = (id & SYSINFO_CHIP_ID_REVISION_BITS) >> SYSINFO_CHIP_ID_REVISION_LSB;


    printf("project: %s\n", DEVICE_PROJECT);
    printf("repo: %s\n", DEVICE_REPO);
    printf("board: %s\n", DEVICE_BOARD);
    printf("serial: %s\n", board_id);
    printf("chip: manufacturer 0x%03x, part 0x%04x, revision %u\n", manufacturer, part, revision);
    printf("pico-sdk: %s", PICO_SDK_VERSION_STRING);
}


void dev_info(void)
{
    //Считаем revision
    volatile  uint32_t *chip_id = (uint32_t *)(SYSINFO_BASE + SYSINFO_CHIP_ID_OFFSET);
    uint32_t id = *chip_id;
    uint8_t revision = (uint8_t)(id & SYSINFO_CHIP_ID_REVISION_BITS) >> SYSINFO_CHIP_ID_REVISION_LSB;



    // Считаем версию
    // uint32_t version_major = ((uint32_t)(uint8_t)*FIRMWARE_VERSION) << 4;
    // uint32_t version_minor = ((uint32_t)(uint8_t)*(FIRMWARE_VERSION+2)) << 2;
    // uint32_t version_patch = ((uint32_t)(uint8_t)*(FIRMWARE_VERSION+4)) << 4;
    // printf("Version %u.%u.%u\n", version_major, version_minor, version_patch);
    // uint32_t version = version_major || version_minor || version_patch;
    uint32_t version = 0x00100000;



    // printf("Processed revision: %u, version: %u\n", revision, version);
    // Пишем структуру
    struct info_t device_card = {
        version,
        DEVICE_NAME,
        revision,
    };
    // device_card.revision =  revision;
    // device_card.version = version;
    // // strcpy(device_card.name, "             ");
    // strcpy(device_card.name, DEVICE_NAME);



    // Все печатаем
    printf("%-15s %-10s %5s %-6s %-13s\n", "struct", "address", "size", "offset", "value");
    printf("%-15s 0x%08x %5u\n",
           "device_card",
           &device_card,
           sizeof(device_card));
    printf("- %-13s 0x%08x %5u %6u 0x%08x\n",
           "version",
           &device_card.version,
           sizeof(device_card.version),
           offsetof(struct info_t, version),
           device_card.version);
    printf("- %-13s 0x%08x %5u %6u %-13s\n",
           "name",
           device_card.name,
           sizeof(device_card.name),
           offsetof(struct info_t, name),
           device_card.name);
    printf("- %-13s 0x%08x %5u %6u 0x%08x\n",
           "revision",
           &device_card.revision,
           sizeof(device_card.revision),
           offsetof(struct info_t, revision),
           device_card.revision);


    unsigned fields = sizeof(device_card.version) + sizeof(device_card.name) + sizeof(device_card.revision);
    printf("fields %u, sizeof %u, padding %u\n", fields, sizeof(device_card), sizeof(device_card) - fields);
}