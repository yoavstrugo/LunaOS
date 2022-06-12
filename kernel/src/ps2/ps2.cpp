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
            if (ioInByte(PS2_PORT_STATUS) & (OUTPUT_BUFFER_STATUS | 0x20) != 0x1)
                return;
    }

    void waitSignal()
    {
        int timeout = 10000;
        while (timeout--)
            if (ioInByte(PS2_PORT_STATUS) & INPUT_BUFFER_STATUS != 0x2)
                return;
    }

    uint8_t executeCommand(uint8_t command)
    {
        // while (pollRead(PS2_PORT_STATUS) & INPUT_BUFFER_STATUS)
        //     ;
        int limit = 3;
        uint8_t response;
        do {
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
#ifdef VERBOSE_PS2
        logDebugn("%! initializing", "[PS2]");
#endif
        // if (!doesPS2Exist())
        // {
        //     kernelPanic("%! PS2 controller does not exist on the system!", "[PS2]");
        //     __builtin_trap();
        // }

        // Disable devices
        waitSignal();
        ioOutByte(PS2_PORT_COMMAND, PS2_CMD_DISABLE_FIRST_PORT);
        waitSignal();
        ioOutByte(PS2_PORT_COMMAND, PS2_CMD_DISABLE_SECOND_PORT);
#ifdef VERBOSE_PS2
        logDebugn("%! Disabled devices", "[PS2]");
#endif

        // Flush output buffer
        while (ioInByte(PS2_PORT_STATUS) & OUTPUT_BUFFER_STATUS)
            ioInByte(PS2_PORT_DATA);

#ifdef VERBOSE_PS2
        logDebugn("%! flushed", "[PS2]");
#endif

        waitSignal();
        ioOutByte(PS2_PORT_COMMAND, PS2_CMD_READ_CONFIGURATION_BYTE);
        waitData();
        uint8_t confByte = pollRead(PS2_PORT_DATA);
        confByte = ((confByte & ~ 0x30) | 3);
        waitSignal();
        ioOutByte(PS2_PORT_COMMAND, PS2_CMD_WRITE_CONFIGURATION_BYTE);
        waitSignal();
        ioOutByte(PS2_PORT_DATA, confByte);
        waitData();
        ioInByte(PS2_PORT_DATA);

        // pitPrepareSleep(1000000);
        // pitPerformSleep();

        // waitSignal();
        // executeCommand(0xF6);
        // waitSignal();
        // executeCommand(0xF4);


#ifdef VERBOSE_PS2
        logDebugn("%! reconfigured", "[PS2]");
#endif

        // Set scancode
        // executeCommand(0xF0);
        // executeCommand(1);

#ifdef VERBOSE_PS2
        logDebugn("%! initialized", "[PS2]");
#endif
    }

    namespace QWERTY
    {
        unsigned char scancode1[] = {
            0,  0x1b, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
            '-', '=', 0x8, 0x9, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I',
            'O', 'P', '[', ']', '\n',  0, 'A', 'S', 'D', 'F', 'G', 'H',
            'J', 'K', 'L', ';', '\'', '`', 0, '\\', 'Z', 'X', 'C', 'V',
            'B', 'N', 'M', ',', '.', '/', 0,  '*'
        };
    }

    void keyboardHandler(uint64_t)
    {
        if (!(ioInByte(PS2_PORT_STATUS) & 1))
            return; // wait for buffer

        uint8_t key = ioInByte(PS2_PORT_DATA);
        // Do not count key release
        if (key < 0x81)
            // Only implemented keys
            if (key < 0x37) {
                k_process *focusedProcess = taskingGetFocusedProcess();
                if (focusedProcess != NULL) {
                    FIL *stdin = focusedProcess->fileDescriptors->get(0);
                    unsigned int byteWritten;
                    f_write(stdin, &QWERTY::scancode1[key], 1, &byteWritten);
                }
            }
    }
}
