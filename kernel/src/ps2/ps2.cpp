#include <ps2/ps2.hpp>

#include <system/acpi/acpi.hpp>
#include <system/acpi/fadt.hpp>
#include <stddef.h>
#include <logger/logger.hpp>
#include <kernel.hpp>
#include <io.hpp>
#include <interrupts/ioapic.hpp>
#include <system/pit.hpp>

#include <tasking/tasking.hpp>

#include <strings.hpp>

namespace PS2
{
    static bool isInitialized = false;

    bool doesPS2Exist()
    {
        FADT *fadt = (FADT *)acpiGetEntryWithSignature("FACP");
        if (fadt == NULL)
        {
            logWarnn("FADT does not exist!");
            return false;
        }

        // If this bit is set, then PS2 exist
        return fadt->bootArchitectureFlags & 0x2;
    }

    void waitData()
    {
        int timeout = 10000;
        while (timeout--)
            if (!(ioInByte(PS2_PORT_STATUS) & 0x1))
                return;
    }

    void waitSignal()
    {
        int timeout = 10000;
        while (timeout--)
            if (!(ioInByte(PS2_PORT_STATUS) & 0x2))
                return;
    }

    uint8_t executeCommand(uint8_t command)
    {
        // while (pollRead(PS2_PORT_STATUS) & INPUT_BUFFER_STATUS)
        //     ;
        int limit = 3;
        uint8_t response;
        do
        {
            ioOutByte(PS2_PORT_COMMAND, command);
            waitData();
            response = ioInByte(PS2_PORT_DATA);
        } while (response == 0xFE && (limit--) > 0);

        return response;
    }

    uint8_t executeCommand(uint8_t command, uint8_t value)
    {
        // while (pollRead(PS2_PORT_STATUS) & INPUT_BUFFER_STATUS)
        //     ;
        ioOutByte(PS2_PORT_DATA, value);
        return executeCommand(command);
    }

    uint8_t pollRead(uint8_t port)
    {
        return ioInByte(port);
    }

    bool initialize()
    {
        // Disable devices
        waitSignal();
        ioOutByte(PS2_PORT_COMMAND, PS2_CMD_DISABLE_FIRST_PORT);
        waitSignal();
        ioOutByte(PS2_PORT_COMMAND, PS2_CMD_DISABLE_SECOND_PORT);

        // Flush output buffer
        while (ioInByte(PS2_PORT_STATUS) & 0x1)
            ioInByte(PS2_PORT_DATA);

        waitSignal();
        ioOutByte(PS2_PORT_COMMAND, PS2_CMD_READ_CONFIGURATION_BYTE);

        waitData();
        uint8_t confByte = ioInByte(PS2_PORT_DATA);
        confByte |= FIRST_PORT_INTERRUPT;
        confByte |= SECOND_PORT_INTERRUPT;
        confByte |= SECOND_PORT_CLOCK;
        confByte |= FIRST_PORT_TRANSLATION;

        waitSignal();
        ioOutByte(PS2_PORT_COMMAND, PS2_CMD_WRITE_CONFIGURATION_BYTE);

        waitSignal();
        ioOutByte(PS2_PORT_DATA, confByte);

        // waitSignal();
        // executeCommand(0xF6);
        // waitSignal();
        // executeCommand(0xF4);

#ifdef VERBOSE_PS2
        logDebugn("%! reconfigured", "[PS2]");
#endif
        // Enable both ports
        waitSignal();
        ioOutByte(PS2_PORT_COMMAND, PS2_CMD_ENABLE_FIRST_PORT);
        waitSignal();
        ioOutByte(PS2_PORT_COMMAND, PS2_CMD_ENABLE_SECOND_PORT);

        // Enable scanning
        // waitSignal();
        // ioOutByte(PS2_PORT_COMMAND, 0xF0);
        // ioOutByte(PS2_PORT_DATA, 1);
        // waitData();
        // uint8_t res = ioInByte(PS2_PORT_DATA);

        // waitSignal();
        // ioOutByte(PS2_PORT_DATA, 1);
        // waitData();
        // res = ioInByte(PS2_PORT_DATA);
        // logInfo("%d", res);

#ifdef VERBOSE_PS2
        logDebugn("%! initialized", "[PS2]");
#endif
    }

    namespace QWERTY
    {
        unsigned char scancode1[] = {
            0, 0x1b, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
            '-', '=', 0x8, '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i',
            'o', 'p', '[', ']', '\n', 0, 'a', 's', 'd', 'f', 'g', 'h',
            'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v',
            'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '};

        unsigned char scancode1Shift[] = {
            0, 0x1b, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
            '_', '+', 0x8, '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I',
            'O', 'P', '{', '}', '\n', 0, 'A', 'S', 'D', 'F', 'G', 'H',
            'J', 'K', 'L', ':', '\"', '~', 0, '|', 'Z', 'X', 'C', 'V',
            'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' '};
    }

    bool leftShift;

    void keyboardHandler(uint64_t)
    {
        if (!(ioInByte(PS2_PORT_STATUS) & 1))
            return; // wait for buffer

        uint8_t key = ioInByte(PS2_PORT_DATA);

        if (key == 0xAA)
        {
            leftShift = false;
            return;
        }

        if (key == 0x2A)
        {
            leftShift = true;
            return;
        }

        // Do not count key release
        if (key < 0x81)

            // Only implemented keys
            if (key < 0x40)
            {
                k_process *focusedProcess = taskingGetFocusedProcess();
                if (focusedProcess != NULL)
                {
                    FIL *stdin = &focusedProcess->stdin;
                    long stdinReadPtr = f_tell(stdin);

                    f_lseek(stdin, focusedProcess->stdinWritePtr);
                    unsigned int bytesWritten;
                    char c = leftShift ? QWERTY::scancode1Shift[key] : QWERTY::scancode1[key];
                    FRESULT res = f_write(stdin, &c, 1, &bytesWritten);

                    focusedProcess->stdinWritePtr += bytesWritten;
                    f_lseek(stdin, stdinReadPtr);
                }
            }
    }
}
