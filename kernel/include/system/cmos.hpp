#pragma once

#include <stdint.h>

#define CMOS_ADDRESS    0x70
#define CMOS_DATA       0x71

/**
 * @brief Represents date time
 * 
 */
struct k_datetime {
    unsigned char second;
    unsigned char minute;
    unsigned char hour;
    unsigned char day;
    unsigned char month;
    unsigned char year;
};

/**
 * @brief Read a byte from the CMOS register
 * 
 * @param cmosRegister The register
 * @return uint8_t The value
 */
uint8_t cmosRead(uint8_t cmosRegister);

/**
 * @brief Write a value to the CMOS register
 * 
 * @param value The value
 */
void cmosWrite(uint8_t value);

/**
 * @brief Get the current day time from the CMOS
 * 
 * @return k_datetime The date time
 */
k_datetime cmosGetDatetime();



