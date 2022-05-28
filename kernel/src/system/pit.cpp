#include <system/pit.hpp>

#include <logger/logger.hpp>
#include <io.hpp>

#define VERBOSE_PIT


static uint32_t timerClocking;
static uint32_t sleepDivisor;

void pitPrepareSleep(uint32_t microseconds)
{
	// Sanity check
	// if (microseconds > 54 * 1000)
	// {
	// 	logWarnn("%! illegal use of sleep. may only sleep up to 54000 microseconds", "[PIT]");
	// 	microseconds = 0;
	// }

	// Disable the speaker
	uint8_t speakerControlByte = ioInByte(0x61);
	speakerControlByte &= ~2; // set bit 1 to 0 (disables speaker output)
	ioOutByte(0x61, speakerControlByte);

	// Initialize PIT
	ioOutByte(0x43, PIT_CHANNEL_2 | PIT_OPMODE_0_IOTC | PIT_ACCESS_LOHIBYTE);

	// Configure PIT, calculate divisor for the requested microseconds
	sleepDivisor = PIT_FREQUENCY / (1000000 / microseconds);

#ifdef VERBOSE_PIT
	logDebugn("%! Prepared sleep of %d microseconds", "[PIT]", microseconds);
#endif
}

void pitPerformSleep()
{
#ifdef VERBOSE_PIT
	logDebugn("%! Started sleep.", "[PIT]");
#endif

	// Write the prepared sleep divisor
	ioOutByte(0x42, sleepDivisor & 0xFF);
	ioOutByte(0x42, sleepDivisor >> 8);

	// Reset the PIT counter and let it start
	uint8_t pitControlByte = ioInByte(0x61);
	ioOutByte(0x61, (uint8_t)pitControlByte & ~1); // clear bit 0
	ioOutByte(0x61, (uint8_t)pitControlByte | 1);  // set bit 0

	// Wait for PIT counter to reach 0
	while (!(ioInByte(0x61) & 0x20))
		;

#ifdef VERBOSE_PIT
	logDebugn("%! Sleep ended.", "[PIT]");
#endif
}