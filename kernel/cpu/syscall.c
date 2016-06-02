#include <stdint.h>
#include "syscall.h"
#include "isr.h"
#include "common.h"
#include "terminal.h"

// We define there can be no more than 128 syscalls
#define SYSCALL_MAX 128
syscall calls[SYSCALL_MAX];

void syscall_callback(registers_t* regs)
{
	// Select the call based on eax. The data to pass to the call is in edx.
	//kprintf("syscall: eax: %x edx: %x\n", regs->eax, regs->edx);
	
	// Temporary implementation to allow for usermode printf()
	if (regs->eax == 0xDEADBEEF)
	{
		// We're trusting user code. This definitely needs to change.
		if (regs->edx != 0)
			kprintf("%s", (char*)regs->edx);
		return;
	}
	
	// Check the requested call is less than SYSCALL_MAX
	if (regs->eax >= SYSCALL_MAX)
	{
		regs->eax = 0;
		return;
	}
	
	// Check a syscall has been registered
	if (calls[regs->eax] == 0)
	{
		regs->eax = 0;
		return;
	}
	
	// If it has, call it and return whatever it returns
	// Also, re-enable interrupts. Syscalls can rely on task switching (sending IPC messages and receiving responses)
	asm volatile("sti");
	uint32_t retval = calls[regs->eax](regs->edx);
	regs->eax = retval;
}

void syscall_init()
{
	reg_int_handler(0x80, &syscall_callback);
	memset((void*)calls, 0, sizeof(syscall)*SYSCALL_MAX);
}

void syscall_register(syscall func, uint32_t call)
{
	calls[call] = func;
}

uint32_t syscall_call(uint32_t call, uint32_t data)
{
	if (calls[call] == 0)
		return 0;
	return calls[call](data);
}