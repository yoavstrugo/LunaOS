#pragma once

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