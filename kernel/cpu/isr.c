#include "common.h"
#include "isr.h"

isr_t int_handlers[256];

void isr_handler(registers_t regs)
{
	if (int_handlers[regs.int_no] != 0)
	{
		isr_t handler = int_handlers[regs.int_no];
		handler(&regs);
	} else {
		kprintf("Unhandled ISR: %i\n", regs.int_no);
		kprintf("Instruction address: %x\n", regs.eip);
		PANIC("Unhandled ISR");
	}
}

void irq_handler(registers_t regs)
{
	if (int_handlers[regs.int_no] != 0)
	{
		isr_t handler = int_handlers[regs.int_no];
		handler(&regs);
	}
	if (regs.int_no >= 40)
	{
		outb(0xA0, 0x20);
	}
	outb(0x20, 0x20);
}

void int_handler(registers_t regs)
{
	if (int_handlers[regs.int_no] != 0)
	{
		isr_t handler = int_handlers[regs.int_no];
		handler(&regs);
	}
}

void reg_int_handler(uint32_t n, isr_t handler)
{
	int_handlers[n] = handler;
}
