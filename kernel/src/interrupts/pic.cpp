#include <interrupts/pic.hpp>

#include <io.hpp>

void picSendEOI(uint8_t irq)
{
	/*
	 If the interrupt is an IRQ (> 32) and on the second PIC (> 8)
	 we have to tell the slave that the interrupt has ended too
	 */
	if (irq >= PIC_SLAVE_OFFSET)
	{
		ioOutByte(PIC2_COMMAND, 0x20);
	}

	ioOutByte(PIC1_COMMAND, 0x20);
}

void picRemapIRQs()
{
	uint8_t a1 = ioInByte(PIC1_DATA); // save masks
	uint8_t a2 = ioInByte(PIC2_DATA);

	ioOutByte(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4); // starts the initialization sequence (in cascade mode)
	ioWait();
	ioOutByte(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
	ioWait();
	ioOutByte(PIC1_DATA, PIC_MASTER_OFFSET); // ICW2: Master PIC vector offset
	ioWait();
	ioOutByte(PIC2_DATA, PIC_SLAVE_OFFSET); // ICW2: Slave PIC vector offset
	ioWait();
	ioOutByte(PIC1_DATA, 4); // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	ioWait();
	ioOutByte(PIC2_DATA, 2); // ICW3: tell Slave PIC its cascade identity (0000 0010)
	ioWait();

	ioOutByte(PIC1_DATA, ICW4_8086);
	ioWait();
	ioOutByte(PIC2_DATA, ICW4_8086);
	ioWait();

	ioOutByte(PIC1_DATA, a1); // restore saved masks.
	ioOutByte(PIC2_DATA, a2);
}

void picSetMask(uint8_t irq)
{
	uint16_t port;
	uint8_t value;

	if (irq < 8)
	{
		port = PIC1_DATA;
	}
	else
	{
		port = PIC2_DATA;
		irq -= 8;
	}
	value = ioInByte(port) | (1 << irq);
	ioOutByte(port, value);
}

void picUnsetMask(uint8_t irq)
{
	uint16_t port;
	uint8_t value;

	if (irq < 8)
	{
		port = PIC1_DATA;
	}
	else
	{
		port = PIC2_DATA;
		irq -= 8;
	}
	value = ioInByte(port) & ~(1 << irq);
	ioOutByte(port, value);
}

void picDisable()
{
	picRemapIRQs();
	// Mask off all the bits
	ioOutByte(PIC1_DATA, 0xFF);
	ioOutByte(PIC2_DATA, 0xFF);
}