#pragma once

#include <stdint.h>

// Commands
#define PS2_CMD_READ_CONFIGURATION_BYTE 0x20
#define PS2_CMD_WRITE_CONFIGURATION_BYTE 0x60

#define PS2_CMD_READ_OUTPUT_PORT 0xD0
#define PS2_CMD_WRITE_OUTPUT_PORT 0xD1

#define PS2_CMD_DISABLE_FIRST_PORT 0xAD
#define PS2_CMD_ENABLE_FIRST_PORT 0xAE
#define PS2_CMD_DISABLE_SECOND_PORT 0xA7
#define PS2_CMD_ENABLE_SECOND_PORT 0xA8

#define PS2_CMD_TEST_SECOND_PORT    0xA9
#define PS2_CMD_TEST_CONTROLLER    0xAA
#define PS2_CMD_TEST_FIRST_PORT    0xAB

#define PS2_CMD_READ_INPUT_PORT    0xC0


// Ports
#define PS2_PORT_DATA 0x60
#define PS2_PORT_STATUS 0x64
#define PS2_PORT_COMMAND 0x64

namespace PS2
{
    enum StatusRegister
    {
        OUTPUT_BUFFER_STATUS = 0x1,
        INPUT_BUFFER_STATUS = 0x2,
        COMMAND_DATA = 0x8,
        TIMEOUT_ERROR = 0x40,
        PARITY_ERROR = 0x80,
    };

    enum ConfigurationByte 
    {
        FIRST_PORT_INTERRUPT = 0x1,
        SECOND_PORT_INTERRUPT = 0x2,
        FIRST_PORT_CLOCK = 0x10,
        SECOND_PORT_CLOCK = 0x20,
        FIRST_PORT_TRANSLATION = 0x40,
    };

    enum OutputPort
    {
        SYSTEM_RESET = 0x1,
        A20_GATE = 0x2,
        SECOND_PORT_CLOCK_OUT = 0x4,
        SECOND_PORT_DATA_OUT = 0x8,
        FIRST_PORT_CLOCK_OUT = 0x40,
        FIRST_PORT_DATA_OUT = 0x80
    };

    /**
     * @brief Initializes the PS/2 controller
     *
     * @return true
     * @return false
     */
    bool initialize();

    /**
     * @brief Execute a command to the Ps/2 controller
     *
     * @param command
     */
    uint8_t executeCommand(uint8_t command);

    uint8_t executeCommand(uint8_t command, uint8_t value);

    void keyboardHandler(uint64_t);
}