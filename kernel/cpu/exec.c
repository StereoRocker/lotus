#include <stdint.h>
#include "exec.h"			// Own header, definition of kexec_jmp
#include "elf.h"			// ELF definitions
#include "common.h"			// kprintf, memset, memcpy
#include "pmm.h"			// pmm_allocpage
#include "vmm.h"			// vmm_set_map
#include "task.h"

// TODO: kexec_elf: Remove all the PANICs and handle those errors in a better way
// TODO: kexec_elf: Allow paging to ensure that non-writeable space in the ELF iamge can't be written to

// Load an ELF file from mem and parse it
// Load it's sections into the current address space
// Jump to the start point in usermode
void kexec_elf(uint8_t* mem, uint32_t length)
{
	asm volatile("cli");
	
	// Ensure that the image given to us is long enough to accomodate at least an ELF header and program header
	if (length < sizeof(elf_header_t) + sizeof(elf_pheader_t))
		PANIC("kexec_elf: file not large enough to be an ELF file");
	
	elf_header_t* elf = (elf_header_t*)mem;
	
	// Ensure this is actually an ELF file
	if (elf->magic[0] != 0x7F)
		PANIC("kexec_elf: first magic field was invalid");
	if (elf->magic[1] != 0x45)
		PANIC("kexec_elf: second magic field was invalid");
	if (elf->magic[2] != 0x4C)
		PANIC("kexec_elf: third magic field was invalid");
	if (elf->magic[3] != 0x46)
		PANIC("kexec_elf: fourth magic field was invalid");
	
	// Ensure this ELF file is for x86
	if (elf->machine != ELF_X86)
		PANIC("kexec_elf: Not for x86");
	
	elf_pheader_t* elf_ph = (elf_pheader_t*)((uint32_t)mem + elf->prog_header);
	
	/*kprintf("Entry point: %x\n", elf->prog_entry);
	kprintf("program headers: %i\n", elf->phead_ents);*/
	
	uint32_t last_addr = 0;
	
	for (uint32_t i = 0; i< elf->phead_ents; i++)
	{
		
		kprintf("vaddr: %x\n", elf_ph->vaddr);
		kprintf("p_offset: %x\n", elf_ph->p_offset);
		kprintf("p_filesz: %x\n", elf_ph->p_filesz);
		kprintf("p_memsz: %x\n", elf_ph->p_memsz);
		
		if (elf_ph->flags & ELF_EXEC)
			kprintf("Exectuable\n");
		if (elf_ph->flags & ELF_WRITE)
			kprintf("Writeable\n");
		if (elf_ph->flags & ELF_READ)
			kprintf("Readable\n"); 
		
		/* We need to do the following steps for each program header:
		 * 1 - Place virtual memory at the requested vaddr large enough to hold up to p_memsz
		 * 1.5 - Ensure the virtual memory has the correct permissions (RW or RO)
		 *     - If the linker script is set up correctly, I can ensure that the code and data lie on different page boundaries. This way virtual memory secures the code from being overwritten!
		 * 2 - Copy p_filesz bytes at p_offset to vaddr
		 */
		
		// Set up virtual memory for this section
		uint32_t pages = elf_ph->p_memsz / 4096;
		if (elf_ph->p_memsz % 4096)
			pages++;
		
		// Allocate the physical pages and page them into virtual memory
		for (uint32_t p = 0; p < pages; p++)
		{
			uint32_t phys = pmm_allocpage();
			ASSERT(phys != 0);
			vmm_set_map(vmm_currentpd(), (void*)(elf_ph->vaddr + (p * 4096)), (void*)phys, PD_USERMODE | PD_RW | PD_REPLACE);	// This needs to be changed to set RO for non-writeable memory
		}
		
		// Set the memory to 0
		memset((uint8_t*)elf_ph->vaddr, 0, elf_ph->p_memsz);
		
		// Copy from the file to memory
		memcpy((uint8_t*)elf_ph->vaddr, (uint8_t*)((uint32_t)mem + elf_ph->p_offset), elf_ph->p_filesz);
		
		// Locate the next program header
		elf_ph = (elf_pheader_t*)((uint32_t)elf_ph + elf->phead_ent_size);
		
		// Calculate the last vaddr of this segment and see if it's the last in the address space
		// TODO: Clean this up
		if (last_addr == 0)
		{
			last_addr = elf_ph->vaddr + elf_ph->p_memsz;
		} else if (last_addr < elf_ph->vaddr + elf_ph->p_memsz) {
			last_addr = elf_ph->vaddr + elf_ph->p_memsz;
		}
	}
	
	// Allocate a 4KiB stack and locate it just below the 1GiB mark (where apps should be loaded)
	// 0x3FFFF000 is the page below the 1GiB mark
	// 0x40000000 should be the new stack pointer, as all values will lie below this
	uint32_t phys = pmm_allocpage();
	ASSERT(phys != 0);
	vmm_set_map(vmm_currentpd(), (void*)(0x3FFFF000), (void*)phys, PD_USERMODE | PD_RW | PD_REPLACE);
	
	// Round the program end and set it for the task
	uint32_t pend = last_addr & 0xFFFFF000;
	if (last_addr % 4096)
		pend += 4096;
	task_setend((uint32_t*)pend);
	
	// This function doesn't return!
	kexec_jmp(0x40000000, elf->prog_entry);
}