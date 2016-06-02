#include <stdint.h>
#include <stdbool.h>
#include "task.h"
#include "heap.h"
#include "pmm.h"
#include "vmm.h"
#include "common.h"
#include "syscall.h"
#include "lotus/syscall.h"
#include "lotus/ipc.h"

task* current;
task* ready;
volatile uint32_t next_pid = 1;
void* stack;
extern uint32_t read_eip();

void copy_stack(void* addr, void* init_sb, uint32_t* pagedir)
{
	// To create a copy of the stack, we need to copy it to a new physical location
	// and then alter the paging map so that it's logically in the same place
	// but physically located elsewhere
	
	// Assume addr is the new physical location for the stack and that it's been
	// identity mapped into the current address space
	// Also assume init_sb is the logical location for the stack
	// Additionally assume the stack is 0x1000 bytes in size
	
	// First: Copy the stack
	memcpy((uint8_t*)addr, (uint8_t*)init_sb, 0x1000);
	
	// Second: Page in the new stack
	vmm_set_map(pagedir, init_sb, addr, PD_RW | PD_USERMODE | PD_REPLACE);
}

void task_init(void* init_sb)
{
	asm volatile("cli");
	
	// We'll copy the stack so we don't overwrite stuff in another task
	//void* addr = (void*)pmm_allocpage();
	//vmm_set_map(vmm_kernelpd(), addr, addr, PD_RW | PD_USERMODE);
	stack = init_sb;
	//copy_stack(addr, init_sb, vmm_kernelpd());
	
	// Initialise the first task (kernel task)
	current = ready = (task*)kmalloc(sizeof(task));
	current->id = next_pid++;
	current->pagedir = vmm_kernelpd();
	current->next = 0;
	current->lock = 0;
	current->msg = 0;
	
	// Also create a new kernel stack for this process
	void* newkstack = (void*)pmm_allocpage();
	vmm_set_map(vmm_kernelpd(), (void*)0xFFFFF000, newkstack, PD_RW | PD_KERNEL);
	ASSERT(newkstack != (void*)0);
	
	asm volatile("sti");
}

// This function should only be directly called from the kernel, so the return
// address should be within the kernel's virtual address space - note that
// syscalls can access this function, but only through the kernel.

// TODO: Modify fork()
//		- Needs to be aware of program breaks
//		- Needs to copy data instead of linking it all (if it even does that idfk, I may even change my mind)
uint32_t fork()
{
	// We're modifying kernel structures so we can't be interrupted
	asm volatile("cli");
	
	// Take a pointer to this process' task struct for later
	task *parent = current;
	
	// Create a new process.
	task *new = (task*)kmalloc(sizeof(task));
	new->id = next_pid++;
	new->next = 0;
	new->lock = 0;
	new->msg = 0;
	
	// Clone the current address space
	uint32_t* pd = vmm_newpd();
	kprintf("fork() pd: 0x%x\n", pd);
	new->pagedir = pd;
	
	// We are the parent, so clone the stack and set up the esp&ebp for our child
	void* newstack = (void*)pmm_allocpage();
	vmm_set_map(vmm_currentpd(), newstack, newstack, PD_RW | PD_KERNEL);
	ASSERT(newstack != (void*)0);
	copy_stack(newstack, stack, pd);
	
	// Also create a new kernel stack for the process
	void* newkstack = (void*)pmm_allocpage();
	vmm_set_map(pd, (void*)0xFFFFF000, newkstack, PD_RW | PD_KERNEL | PD_REPLACE);
	ASSERT(newkstack != (void*)0);
	
	// Add it to the end of the ready queue
	task *tmp = ready;
	while (tmp->next)
		tmp = tmp->next;
	tmp->next = new;
	
	// Force the switch to the new task, so both tasks have stored EIPs
	task_forceswitch(new, 1);
	
	// We could be the parent or the child here - check.
	if (current == parent)
	{
		// Finished: re-enable interrupts
		asm volatile("sti");
		return new->id;
	}
	new->next=0;
	// We are the child, by convention return 0
	asm volatile("sti");
	return 0;
}

void task_forceswitch(task* t, uint8_t newtask)
{
	// We're fucking around with shit, so stop shit happening
	asm volatile("cli");
	//bochs_break();
	
	// First, store this task's esp & abp
	uint32_t esp;
	uint32_t ebp;
	asm volatile("mov %%esp, %0" : "=r"(esp));
	asm volatile("mov %%ebp, %0" : "=r"(ebp));
	current->esp = esp; current->ebp = ebp;
	
	// Set the next task
	current = t;
	
	// Change pagedir, esp&ebp
	uint32_t pd_addr = (uint32_t)(current->pagedir);
	esp = current->esp;
	ebp = current->ebp;
	if (newtask == 0)
	{
		asm volatile(	"mov %0, %%cr3\n"	// pd_addr
						"mov %1, %%esp\n"	// esp
						"mov %2, %%ebp\n"	// ebp
			: : "r" (pd_addr), "r" (esp), "r" (ebp));
	} else { asm volatile("mov %0, %%cr3" : : "r" (pd_addr)); }
	
	// We're done switching tasks, so clear the timer
	outb(0x20, 0x20);
}

void task_switch()
{
	// If we haven't initialised tasking yet, just return
	if (!current)
	{
		return;
	}
	
	// If there are no other ready tasks, skip this procedure.
	if ((current->next == 0) && (ready == current))
	{
		//kprintf("No other tasks to run\n");
		return;
	}
	
	task* t;
	// Calculate the next task
	if (current->next == 0)
		t = ready;
	else
		t = current->next;
	
	// Switch tasks
	task_forceswitch(t, 0);
	asm volatile("sti");
}

uint32_t getpid()
{
	return current->id;
}

uint8_t task_ready()
{
	if (!current)
		return 0;
	else
		return 1;
}

/* IPC routines */
void ipc_send(uint32_t pid, uint8_t* data, uint32_t length)
{
	// First check we can find the destination pid, and store a reference to it
	task* t = ready;
	while (true)
	{
		if (t->id == pid)
			break;
		
		if (t->next == 0)
		{
			return;
		}
		
		t = t->next;
	}
	
	// Then we need to allocate space for the message and it's data
	message* kmsg = (message*)kmalloc(sizeof(message));
	uint8_t* mdata = (uint8_t*)kmalloc(length);
	
	// Ensure both allocations worked
	// This section will need to be refined, it's not good to panic
	// upon IPC failure as this be maliciously used to crash the kernel
	if (kmsg == 0)
	{
		PANIC("Couldn't allocate space for IPC message header");
	}
	
	if (mdata == 0)
	{
		PANIC("Couldn't allocate space for IPC message data");
	}
	
	// Create the message
	kmsg->next = 0;
	kmsg->sender_id = getpid();
	kmsg->length = length;
	kmsg->data = mdata;
	
	// Copy the data into kernel space
	memcpy((uint8_t*)mdata, (uint8_t*)data, kmsg->length);
	
	// Add this message to the end of the receiving process' IPC queue
	
	// First, lock the task's mutex as we're modifying it
	mutex_lock(&(t->lock));
	
	// Now check if a message already exists. If not, this message becomes the top of the queue
	if (t->msg == 0)
	{
		t->msg = kmsg;
	} else {
		// Find the end of the message queue and add this message
		message* m = t->msg;
		while (m->next != 0)
		{
			m = m->next;
		}
		m->next = kmsg;
	}
	
	// Unlock the task's mutex and return
	mutex_unlock(&(t->lock));
}

int32_t ipc_msglen()
{
	if (current->msg == 0)
		return -1;
	return current->msg->length;
}

uint8_t ipc_getdata(uint8_t* buf)
{
	if (current->msg == 0)
		return 0;
	
	memcpy(buf, current->msg->data, current->msg->length);
	return 1;
}

uint32_t ipc_senderpid()
{
	if (current->msg == 0)
		return 0;
	return current->msg->sender_id;
}

void ipc_nextmsg()
{
	// Reference our task locally
	task* t = current;
	
	// Lock the mutex
	mutex_lock(&(t->lock));
	
	// Ensure a message is present
	if (t->msg != 0)
	{
		// Reference the message
		message* m = t->msg;
		
		// Change the task's reference
		t->msg = t->msg->next;
		
		// Free the data
		kfree(m->data);
		kfree(m);
	}
	
	// Unlock the mutex, and we're done
	mutex_unlock(&(t->lock));
}

/* Syscall stuff */
void task_setend(uint32_t* end)
{
	current->prog_end = end;
}

uint32_t* task_getend()
{
	return current->prog_end;
}

// Whoops, this wasn't compliant! return value must be the start of the new memory
uint32_t sbrk_sys(uint32_t incr)
{
	//kprintf("sbrk called with parameter: %x\n", incr);
	
	// If incr == 0, return the current program end
	if (incr == 0)
		return (uint32_t)task_getend();
	
	// We need to get the current end of the data segment
	uint32_t pend_prev = (uint32_t)task_getend();
	uint32_t pend_new;
	
	// Calculate how many more pages need to be added to the address space
	uint32_t pages = incr / 4096;
	if (incr % 4096)
		pages++;
	
	// Allocate and add the pages
	for (uint32_t i = 0; i < pages; i++)
	{
		// Allocate the page
		uint32_t phys = pmm_allocpage();
		ASSERT(phys != 0);
		
		// Add it to the address space
		vmm_set_map(vmm_currentpd(), (void*)(pend_prev + (i*4096)), (void*)phys, PD_RW | PD_USERMODE);
	}
	
	// Calculate the new program end
	pend_new = pend_prev + (pages * 4096);
	
	// Set the new program end
	task_setend((uint32_t*)pend_new);
	
	// Return the location of the new memory (the old program end)
	return pend_prev;
}

uint32_t ipc_send_sys(uint32_t data)
{
	// This structure comes from user mode! Be careful with the data.
	ipcsend_t* ipcreq = (ipcsend_t*)data;
	
	// This may be insecure. I'm not sure.
	ipc_send(ipcreq->target_pid, ipcreq->data, ipcreq->length);
	
	// Return a value
	return 0;
}

uint32_t ipc_msglen_sys(uint32_t data)
{
	// The data pointer bears no meaning to this function
	UNUSED(data);
	
	// Return the result of the function
	return (uint32_t)ipc_msglen();
}

uint32_t ipc_getdata_sys(uint32_t data)
{
	// The data pointer points to the user mode buffer
	
	// Ideally we wouldn't trust that the buffer is pointing to the correct location
	return (uint32_t)ipc_getdata((uint8_t*)data);
}

uint32_t ipc_nextmsg_sys(uint32_t data)
{
	// Data and return values are meaningless
	UNUSED(data);
	
	ipc_nextmsg();
	return 0;
}

uint32_t ipc_senderpid_sys(uint32_t data)
{
	// Data value is meaningless
	UNUSED(data);
	
	// Return value is the result of ipc_senderpid()
	return ipc_senderpid();
}

uint32_t fork_sys(uint32_t data)
{
	// Data value is meaningless
	UNUSED(data);
	
	// Return the value of fork()
	// TODO: Ensure fork() works correctly. It must copy data pages instead of linking them
	// For now, we deal with whatever happens
	return fork();
}

uint32_t exit_sys(uint32_t data)
{
	// Data and exit values are meaningless - especially since this function should never return
	UNUSED(data);
	
	// For now we just spin forever. We'll get to cleaning up the address space and freeing memory, and unlinking from the task chain later
	for(;;);
}

uint32_t yield_sys(uint32_t data)
{
	// Data and return values are meaningless
	UNUSED(data);
	
	// Yield our timeslice to another task
	task_switch();
	
	return 0;
}

void task_regsyscalls()
{
	// We will define 8 system calls referring to tasking & IPC here
	syscall_register(sbrk_sys, SYS_TASKSBRK);
	syscall_register(fork_sys, SYS_TASKFORK);
	syscall_register(exit_sys, SYS_TASKEXIT);
	syscall_register(yield_sys, SYS_TASKYIELD);
	
	syscall_register(ipc_send_sys, SYS_IPCSEND);
	syscall_register(ipc_msglen_sys, SYS_IPCMSGLEN);
	syscall_register(ipc_getdata_sys, SYS_IPCGETDATA);
	syscall_register(ipc_senderpid_sys, SYS_IPCSENDERPID);
	syscall_register(ipc_nextmsg_sys, SYS_IPCNEXTMSG);
}