#pragma once

#include <stdint.h>

// PIC constants
#define PIC1			0x20 // Master
#define PIC2			0xA0 // Slave
#define PIC1_COMMAND	PIC1
#define PIC1_DATA		(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA		(PIC2+1)
#define ICW1_ICW4	0x01		/* ICW4 (not) needed */
#define ICW1_SINGLE	0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL	0x08		/* Level triggered (edge) mode */
#define ICW1_INIT	0x10		/* Initialization - required! */
 
#define ICW4_8086	0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
#define ICW4_SFNM	0x10		/* Special fully nested (not) */

#define PIC_MASTER_OFFSET   0x20
#define PIC_SLAVE_OFFSET    0x28


/**
 * @brief Remaps all the IRQS on the 2 PICs so it wont collide with exceptions
 * 
 */
void picRemapIRQs();

/**
 * @brief Send the End Of Interrupt signal to the PIC
 * 
 * @param irq The IRQ number
 */
void picSendEOI(uint8_t irq);

/**
 * @brief Set the mask the the irq
 * 
 * @param irq The irq
 */
void picSetMask(uint8_t irq);
 
/**
 * @brief Unmask the the irq
 * 
 * @param irq The irq
 */
void picUnsetMask(uint8_t irq);