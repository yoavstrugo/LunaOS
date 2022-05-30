#pragma once

#include <stdint.h>

#define CMOS_ADDRESS    0x70
#define CMOS_DATA       0x71


uint8_t cmosRead(uint8_t cmosRegister);

void cmosWrite(uint8_t value);

struct k_datetime {
    unsigned char second;
    unsigned char minute;
    unsigned char hour;
    unsigned char day;
    unsigned char month;
    unsigned char year;
};

k_datetime cmosGetDatetime();