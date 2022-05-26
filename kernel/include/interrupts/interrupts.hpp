#pragma once

#include <stdint.h>
#include <tasking/tasking.hpp>

void interruptsInitialize();

/**
 * @brief Installs all of the interrupts
 *
 */
void interruptsInstallRoutines();

/**
 * @brief Disable the interrupts.
 *
 */
void interruptsDisable();

/**
 * @brief Enable the interrupts.
 *
 */
void interruptsEnable();

extern "C" k_thread_state * interruptHandler(k_thread_state *rsp);