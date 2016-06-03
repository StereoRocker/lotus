#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <lotus/syscall.h>
#include <lotus/ipc.h>
#include <lotus/vfs.h>

uint32_t syscall(uint32_t call, uint32_t data)
{
	uint32_t ret;
	asm volatile("mov %1, %%eax; mov %2, %%edx; int $0x80; mov %%eax, %0;" : "=r"(ret) : "r"(call), "r"(data));
	return ret;
}

void ipc_send(uint32_t pid, uint32_t length, uint8_t* data)
{
	// Attempt to send an IPC request
	
	// Set up the system call packet
	ipcsend_t* ipcreq = malloc(sizeof(ipcsend_t));
	ipcreq->target_pid = pid;
	ipcreq->length = length;
	ipcreq->data = data;
	
	// Execute the system call
	syscall(SYS_IPCSEND, (uint32_t)ipcreq);
	
	// Free the memory
	free(ipcreq);
}

int32_t ipc_msglen()
{
	int32_t ret = (int32_t)syscall(SYS_IPCMSGLEN, 0);
	return ret;
}

uint8_t ipc_getdata(uint8_t* buffer)
{
	uint8_t ret = (uint8_t)syscall(SYS_IPCGETDATA, (uint32_t)buffer);
	return ret;
}

void ipc_nextmsg()
{
	syscall(SYS_IPCNEXTMSG, 0);
}

uint32_t ipc_senderpid()
{
	return syscall(SYS_IPCSENDERPID, 0);
}

void yield()
{
	syscall(SYS_TASKYIELD, 0);
}

void main()
{
	printf("Testdev executing\n");
	for(;;);
}

// It's not ideal that we still have to use this, but hopefully it will be gone
// when I compile GCC to use my port of newlib.
extern void _exit();
void _start()
{
	main();
	_exit(0);
}