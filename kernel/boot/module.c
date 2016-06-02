#include <stdint.h>
#include "common.h"
#include "module.h"
#include "multiboot.h"
#include "pmm.h"
#include "vmm.h"
#include "heap.h"

uint32_t mcount;
mod_info_t* minfo;

// Marks the physical address spaces of the modules as used
void mod_initpmm(multiboot_t* multiboot)
{
	if (!(multiboot->flags & MULTIBOOT_MODS))
	{
		PANIC("No modules found!");
	}
	
	kprintf("mod_initpmm: Module count: %i\n", multiboot->mods_count);
	kprintf("mod_initpmm: Module information: %x\n\n", multiboot->mods_addr);
	uint32_t* mod_start;
	uint32_t* mod_end;
	uint32_t mod_len;
	for (uint32_t i = 0; i < multiboot->mods_count; i++)
	{
		// Calculate the addresses of mod_start and mod_end
		mod_start = (uint32_t*)(multiboot->mods_addr + (i*16));
		mod_end = (uint32_t*)((uint32_t)mod_start + 4);
		
		// Print out information about the module
		kprintf("mod_initpmm: Module %i start: %x\n", i, *mod_start);
		kprintf("mod_initpmm: Module %i end: %x\n", i, *mod_end);
		
		// Align the module's start address
		mod_start = (uint32_t*) ((uint32_t)(*mod_start) & 0xFFFFF000);
		
		// Calculate the module's length in pages
		mod_len = *mod_end - (uint32_t)mod_start;
		if (mod_len % 4096)
			mod_len = (mod_len/4096) +1;
		else
			mod_len /= 4096;
		
		kprintf("mod_initpmm: Module %i length (pages, dec): %i\n\n", i, mod_len);
		
		// Work directly with the PMM to mark the module space as used
		for (uint32_t j = 0; j < mod_len; j++)
		{
			pmm_setbit(((uint32_t)mod_start/4096) + j, PMM_MEMUSED);
		}
	}
}

// Pages the modules into memory, and allocates objects on the heap to keep track of them more easily
void mod_initvmm(multiboot_t* multiboot)
{
	uint32_t* mod_start;
	uint32_t* mod_end;
	uint32_t mod_len;
	
	/*
	// Calculate the number of pages that are required to read the module information
	mod_start = (uint32_t*)multiboot->mods_addr;
	mod_len = multiboot->mods_count * 16;
	mod_len += ((uint32_t)mod_start % 4096);
	mod_start = (uint32_t*) ((uint32_t)mod_start & 0xFFFFF000);
	mod_len /= 4096;
	
	// Identity map the pages into memory
	for (uint32_t i = (uint32_t)mod_start; i < (uint32_t)mod_start + mod_len; i+=4096)
	{
		vmm_set_map(vmm_kernelpd(), (void*)i, (void*)i, PT_RW | PT_KERNEL);
	}
	*/
	
	// Allocate enough space to store info about all the modules
	mcount = multiboot->mods_count;
	minfo = (mod_info_t*)kmalloc(sizeof(mod_info_t) * mcount);
	
	// Store the info about the modules and identity map them
	for (uint32_t i = 0; i < multiboot->mods_count; i++)
	{
		// Calculate the addresses of mod_start and mod_end
		mod_start = (uint32_t*)(multiboot->mods_addr + (i*16));
		mod_end = (uint32_t*)((uint32_t)mod_start + 4);
		
		// Store the module's actual start address
		minfo[i].mod_start = (uint32_t*)*mod_start;
		
		// Store the module's actual length
		minfo[i].mod_length = (uint32_t)(*mod_end) - (uint32_t)(*mod_start);
		
		// Align the module's start address
		mod_start = (uint32_t*) ((uint32_t)(*mod_start) & 0xFFFFF000);
		
		// Calculate the module's length in pages
		mod_len = *mod_end - (uint32_t)mod_start;
		if (mod_len % 4096)
			mod_len = (mod_len/4096) +1;
		else
			mod_len /= 4096;
		
		// Identity map the module
		for (uint32_t j = (uint32_t)mod_start; j < (uint32_t)mod_start + (mod_len*4096); j+= 4096)
		{
			vmm_set_map(vmm_kernelpd(), (void*)j, (void*)j, PT_RW | PT_KERNEL);
		}
	}
}

// Returns the number of modules
uint32_t mod_count()
{
	return mcount;
}

// Returns the starting address of the module at index
uint32_t* mod_addr(uint32_t index)
{
	ASSERT(index < mcount);
	return minfo[index].mod_start;
}

// Returns the length of the module at index
uint32_t mod_len(uint32_t index)
{
	ASSERT(index < mcount);
	return minfo[index].mod_length;
}