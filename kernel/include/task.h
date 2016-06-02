#ifndef TASK_H
#define TASK_H

#include <stdint.h>
#include "mutex.h"

typedef struct message {
	struct message *next;
	uint32_t sender_id;
	uint32_t length;
	uint8_t* data;
} message;

typedef struct task {
	uint32_t id;
	uint32_t esp, ebp;
	uint32_t* pagedir;
	uint8_t* kstack;
	uint32_t* prog_end;
	struct task *next;
	
	struct message *msg;
	mutex lock;
} task;

/* Tasking routines */
void task_init();
void task_switch();										// Same as yield();
void task_forceswitch(task* t, uint8_t newtask);
uint32_t fork();
uint32_t getpid();
uint8_t task_ready();
void task_setend(uint32_t* end);

/* Note that when the function specified by addr returns, the code
 * after this function is called will be executed in usermode as well! */
void task_jumptouser(uint32_t addr);

/* IPC routines */
void ipc_send(uint32_t pid, uint8_t* data, uint32_t length);
int32_t ipc_msglen();
uint8_t ipc_getdata(uint8_t* buf);					// Returns 0 on error
uint32_t ipc_senderpid();							// Returns 0 on error
void ipc_nextmsg();

/* System call shenanigans */
void task_regsyscalls();							// Registers syscalls

#endif
