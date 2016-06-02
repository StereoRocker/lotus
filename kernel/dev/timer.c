#include "timer.h"
#include "isr.h"
#include "common.h"
#include "task.h"

volatile uint64_t ticks = 0;

void timer_callback(registers_t* regs)
{
	// This is actually to stop a compiler warning
	regs->int_no = regs->int_no;
	ticks++;
	
	// We switch tasks every 5ms
	if (ticks%5 == 0)
		task_switch();
}

#define TIMER_FREQ 1193180
void timer_init(uint32_t freq)
{
	uint32_t divisor = TIMER_FREQ / freq;
	uint8_t  lo = (uint8_t)(divisor & 0xFF);
	uint8_t  hi = (uint8_t)((divisor>>8) & 0xFF);
	
	// Set the IRQ handler
	reg_int_handler(IRQ0, &timer_callback);
	
	// Send the command byte
	outb(0x43, 0x36);
	// Send the frequency divisor
	outb(0x40, lo);
	outb(0x40, hi);
}

uint64_t timer_getticks()
{
	return ticks;
}
