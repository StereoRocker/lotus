#include <stdint.h>
#include "terminal.h"
#include "multiboot.h"
#include "common.h"
#include "pmm.h"
#include "dt.h"
#include "isr.h"
#include "timer.h"
#include "vmm.h"
#include "task.h"
#include "heap.h"
#include "syscall.h"
#include "module.h"
#include "exec.h"
#include "fs.h"

void kmain(multiboot_t* multiboot, void* init_sb)
{
	// Terminal shizz (we have a working kprintf!)
	term_init();
	kprintf("Greetings from Lotus, kernel world!\n");
	
	// Multiboot shizz
	kprintf("Multiboot header address: 0x%x\n", multiboot);
	if (multiboot->flags & MULTIBOOT_MEM)
		kprintf("Lower memory: %iKiB\nUpper memory: %iKiB\n",
		multiboot->mem_lower, multiboot->mem_upper);
	
	if (multiboot->flags & MULTIBOOT_CMDLINE)
		kprintf("cmdline: %s\n", (char*)(multiboot->cmdline));
	
	// Initialise physical memory manager
	ASSERT(multiboot->flags & MULTIBOOT_MMAP);
	pmm_init(multiboot->mmap_length, multiboot->mmap_addr);
	
	// Deal with modules and mark their address spaces as used
	mod_initpmm(multiboot);
	
	// Initialise descriptor tables
	dt_init();
	set_kstack(0xFFFFFFFF);
	
	// Initialise timer
	timer_init(1000);
	
	// Initialise virtual memory manager
	vmm_init();
	
	// Initialise the kernel heap
	heap_init();
	
	// Page modules into memory
	mod_initvmm(multiboot);
	
	// Initialise tasking
	task_init(init_sb);
	
	// Initialise system calls
	syscall_init();
	
	// Initialise system calls for tasking
	task_regsyscalls();
	
	// Initialise VFS (initialises devfs as well)
	vfs_init();
	
	// Initialise device system (unless it becomes built into the VFS)
	
	// Test our brand spanking new kexec_elf function!
	// It loads an ELF into the current address space and executes it in user mode
	// The ELF is passed as a pointer to a memory buffer, with a length
	// We're going to fork before loading it, as we will want to query the VFS in kernel land
	ASSERT(mod_count() > 0);
	
	for (int i = 1; i < 2; i++)
	{
		uint32_t pid = fork();
		if (!pid)
			kexec_elf((uint8_t*)mod_addr(i), mod_len(i));
	}
	
	/*for (int i = 0; i < 16; i++)
	{
		uint32_t pid = fork();
		if (!pid)
		{
			pid = getpid();
			while (1)
			{
				kprintf("Child %i executing\n", pid);
				task_switch();
			}
		}
	}*/
	
	// Wait for the VFS to register a filesystem
	while (1)
	{
		uint32_t count = vfs_fscount();
		if (count > 0)
			break;
	}
		
	kprintf("VFS registers %i filesystem(s)\n", vfs_fscount());
		
	// Test mounting
	uint32_t ret = vfs_mount("/", "none", "dummy");
		
	kprintf("Mounting none on / with dummy returned: %x\n", ret);
		
	// Test opening
	ret = vfs_open("/blarg");
	kprintf("vfs_open returned %x\n", ret);
		
	// Test reading
	uint32_t data = 0;
	ret = vfs_read("/blarg", 0, 4, (uint8_t*)&data);
	kprintf("vfs_read returned reading %i bytes, which were: %x\n", ret, data);
	
	// Execute IRD, IFS, mount IFS on IRD
	/* fork
	 * If child: exec(IRD)
	 * If parent: continue below
	 *
	 * fork
	 * If child: exec(IFS)
	 * If parent: continue below
	 *
	 * Poll device system and FS system, waiting for IRD and IFS to register themselves
	 * Mount IFS on IRD
	 */
	
	// Unmap modules from paging space and mark the space they take as free
	//mod_free();
	
	// Attempt to execute /sbin/init (without forking this task - we'll replace the address space instead)
	
	// Spin and try not to exit back to the start of the kernel
	for (;;);
}
