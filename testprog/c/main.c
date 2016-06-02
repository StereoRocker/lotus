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

/*void ipc_test()
{
	// call ipc_msglen() and print it's result
	
	// Busy wait for ipc_msglen() to return something other than -1
	while (ipc_msglen() < 0)
	{};
	
	// Get the result and display it
	int32_t ret = ipc_msglen();
	printf("ipc_msglen() returned: %i\n", ret);
	
	// Now allocate a buffer large enough to contain the data, and retrieve it!
	char* buf = malloc(ret);
	ipc_getdata((uint8_t*)buf);
	
	// Output what was sent to us and by who
	printf("PID %i sent message: %s\n", ipc_senderpid(), buf);
	
	// Now tell the IPC system we're done with the message
	ipc_nextmsg();
	
	// Free the buffer
	free(buf);
}*/

void yield()
{
	syscall(SYS_TASKYIELD, 0);
}

char* fsName = "dummy";
uint8_t mounted = 0;

void main()
{
	// Register as a filesystem
	syscall(SYS_VFSREG, (uint32_t)fsName);
	
	// Busy-wait for an IPC packet, yielding when none has arrived
	while (1)
	{
		while (ipc_msglen() < 0)
		{ yield(); }
		
		// There is a message available
		int32_t ret = ipc_msglen();
		
		// Allocate a buffer to hold it
		uint8_t* buf = malloc(ret);
		ipc_getdata(buf);
		
		// Interpret the message
		fsreq_t* req = (fsreq_t*)buf;
		
		// Ensure the magic field is correct
		if (req->magic != FS_REQ_MAGIC)
		{
			// Free the buffer
			free(buf);
			
			// Ask the IPC subsystem for the next message
			ipc_nextmsg();
			
			// Continue busy waiting
			continue;
		}
		
		// Print the request
		printf("IFS: Request %X\n", req->request);
		
		// Get the PID of the sender
		uint32_t pid = ipc_senderpid();
		
		// Declare a pointer to a response variable. Each case will allocate and free this
		fsres_t* res;
		
		// Perform an action based on the request
		switch (req->request)
		{
			case FS_MOUNT:
				// If we were to develop a fully featured FS driver, this function would do more
				
				// For now, we check if we've already been mounted
				// If we have, return failure
				if (mounted)
				{
					// This is the failure clause
					
					// Set up the response packet
					res = malloc(sizeof(fsres_t));
					res->magic = FS_RES_MAGIC;
					res->success = 0;
					res->length = 0;
					
					// Send the response packet
					ipc_send(pid, sizeof(fsres_t), (uint8_t*)res);
					
					// Free the response packet
					free(res);
					
					// Break from the switch to clean-up
					break;
				}
				
				// Otherwise, return success and an fsID of 0
				
				// Set mounted as 1
				mounted = 1;
				
				// Set up the response packet
				res = malloc(sizeof(fsres_t) + 1);
				uint8_t* fsID = (uint8_t*)((uint32_t)res + sizeof(fsres_t));
				res->magic = FS_RES_MAGIC;
				res->success = 1;
				res->length = 1;
				*fsID = 0;
				
				// Send the response packet
				ipc_send(pid, sizeof(fsres_t)+1, (uint8_t*)res);
				
				// Free the response packet
				free(res);
				
				// Break from the switch to clean-up
				break;
			case FS_UNMOUNT:
				// If we were to develop a fully featured FS driver, this function would do more
				
				// For now we check if we've already been mounted
				// If we have, mark ourselves as unmounted and return success
				// Otherwise, return failure
				if (mounted)
				{
					// This is the success clause
					
					// Set mounted as 0
					mounted = 0;
					
					// Set up the response packet
					res = malloc(sizeof(fsres_t));
					res->magic = FS_RES_MAGIC;
					res->success = 1;
					res->length = 0;
					
					// Send the response packet
					ipc_send(pid, sizeof(fsres_t), (uint8_t*)res);
					
					// Free the response packet
					free(res);
					
					// Break from the switch to clean-up
					break;
				}
				
				// Otherwise, return failure
				
				// Set up the response packet
				res = malloc(sizeof(fsres_t));
				res->magic = FS_RES_MAGIC;
				res->success = 0;
				res->length = 0;
					
				// Send the response packet
				ipc_send(pid, sizeof(fsres_t), (uint8_t*)res);
					
				// Free the response packet
				free(res);
				
				// Break from the switch to clean-up
				break;
			case FS_CLOSE:
			case FS_OPEN:
				// Check the file exists.
				// In the real IFS, we'll actually have to check the path against
				// the filesystem entries. To make things simpler, we won't support
				// subdirectories.
				
				// For now, we'll just say it exists
				
				// Set up the response packet
				res = malloc(sizeof(fsres_t));
				res->magic = FS_RES_MAGIC;
				res->success = 1;
				res->length = 0;
					
				// Send the response packet
				ipc_send(pid, sizeof(fsres_t), (uint8_t*)res);
					
				// Free the response packet
				free(res);
				
				break;
			case FS_READ:
				// We need a test case
				// For now we can return 0xCAFEBABE
				
				// Set up the response packet
				res = malloc(sizeof(fsres_t) + 4);
				res->magic = FS_RES_MAGIC;
				res->success = 1;
				res->length = 4;
				
				// Add the contents
				uint32_t* number = (uint32_t*)((uint32_t)res + sizeof(fsres_t));
				*number = 0xCAFEBABE;
					
				// Send the response packet
				ipc_send(pid, sizeof(fsres_t) + 4, (uint8_t*)res);
					
				// Free the response packet
				free(res);
				
				break;
			case FS_WRITE:
				// The IFS won't support writing, so we'll return failure
				// Set up the response packet
				res = malloc(sizeof(fsres_t));
				res->magic = FS_RES_MAGIC;
				res->success = 0;
				res->length = 0;
					
				// Send the response packet
				ipc_send(pid, sizeof(fsres_t), (uint8_t*)res);
					
				// Free the response packet
				free(res);
				break;
			case FS_FILESIZE:
				// Let's just not test this until we've implemented a real IFS
				printf("Filesize was requested and hasn't been implemented\n");
				printf("Sorry for hanging the system! :D\n");
				break;
		}
		
		// Free the buffer
		free(buf);
		
		// Ask the IPC subsystem for the next message
		ipc_nextmsg();
		
		// Continue busy waiting
	}
}

// It's not ideal that we still have to use this, but hopefully it will be gone
// when I compile GCC to use my port of newlib.
extern void _exit();
void _start()
{
	main();
	_exit(0);
}