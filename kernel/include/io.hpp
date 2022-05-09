#pragma once

#include <stdint.h>

/**
 * @brief Writes the value into the port
 * 
 * @param port The port
 * @param value The value
 */
void ioOutByte(uint16_t port, uint8_t value);

/**
 * @brief Reads the value from the port
 * 
 * @param port The port 
 * @return uint8_t The value
 */
uint8_t ioInByte(uint16_t port);

void ioWait();