#pragma once

#include <stdint.h>

#define PIT_FREQUENCY   1193180

#define PIT_CHANNEL_0				0x00	// 00......
#define PIT_CHANNEL_1				0x40	// 01......
#define PIT_CHANNEL_2				0x80	// 10......
#define PIT_CHANNEL_READBACK		0xC0	// 11......
#define PIT_ACCESS_LATCHCOUNT		0x00	// ..00....
#define PIT_ACCESS_LOBYTE			0x10	// ..01....
#define PIT_ACCESS_HIBYTE			0x20	// ..10....
#define PIT_ACCESS_LOHIBYTE			0x30	// ..11....
#define PIT_OPMODE_0_IOTC			0x00	// ....000.
#define PIT_OPMODE_1_ONESHOT		0x02	// ....001.
#define PIT_OPMODE_2_RATE_GEN		0x04	// ....010.
#define PIT_OPMODE_3_SQUARE_WAV		0x06	// ....011.
#define PIT_OPMODE_4_SOFTWARESTROBE	0x08	// ....100.
#define PIT_OPMODE_4_HARDWARESTROBE	0x0A	// ....101.
#define PIT_OPMODE_4_RATE_GEN		0x0C	// ....110.
#define PIT_OPMODE_4_SQUARE_WAV		0x0E	// ....111.
#define PIT_BINARY					0x00	// .......0
#define PIT_BCD						0x01	// .......1

/**
 * Prepares the PIT to sleep for the specified number of
 * microseconds when the 'performSleep()' function is called.
 *
 * @param microseconds 	number of microseconds to sleep
 */
void pitPrepareSleep(uint32_t microseconds);

/**
 * Performs sleep that was before configured using the {prepareSleep}
 * function. A sleep can executed as often as wished onced its prepared.
 */
void pitPerformSleep();