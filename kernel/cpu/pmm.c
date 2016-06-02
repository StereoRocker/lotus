#include <stdint.h>
#include "common.h"
#include "terminal.h"
#include "multiboot.h"
#include "pmm.h"
#include "vmm.h"

/* Structure:
 * 1 bit in map = 4096 bytes (ie 1 page)
 * if bit is set, memory is available
 * if bit is not set, memory is used or reserved
 */
uint8_t *mmap;
uint32_t mmaplen;
extern uint32_t begin;
extern uint32_t end;

void pmm_init(uint32_t len, uint32_t addr)
{
	// Find the last usable address in memory
	uint32_t read = 0;
	uint32_t last_addr = 0;
	mmap_entry_t* mement = (mmap_entry_t*)addr;
	while (read < len)
	{
		//kprintf("type: %i base 0x%x\tlength 0x%x\n", mement->type, mement->base, mement->length);
		if (mement->type == 1 && (mement->base + mement->length) > last_addr)
			last_addr = (mement->base + mement->length);
		read += mement->size + 4;
		mement = (mmap_entry_t*)(addr + read);
	}
	
	// Calculate how many bytes are required to map the entirety of usable
	// memory, and initialise the map
	kprintf("Last available address 0x%x\n", last_addr);
	uint32_t bitsreq = last_addr / 4096;
	uint32_t bytesreq = bitsreq / 8;
	if (bitsreq % 8)
		bytesreq++;
	kprintf("bits: %i\tbytes: %i\n", bitsreq, bytesreq);
	mmap = (uint8_t*)((last_addr - bytesreq) & PAGE_PADDR_MASK);	// Ensure it is page aligned
	mmaplen = bytesreq;
	kprintf("mmap: 0x%x\n", (uint32_t)mmap);
	
	// Set the free memory in the map
	read = 0;
	mement = (mmap_entry_t*)addr;
	while (read < len)
	{
		if (mement->type == 1)
		{
			for (uint32_t i = mement->base; i < mement->base + mement->length; i+=4096)
			{
				pmm_setbit(i/4096, PMM_MEMFREE);
				ASSERT(pmm_getbit(i/4096) == PMM_MEMFREE);
			}
		}
		read += mement->size + 4;
		mement = (mmap_entry_t*)(addr + read);
	}
	
	// Ensure that the memory the kernel occupies is marked as used
	// Including the memory before the kernel
	void* baddr = &begin;
	void* eaddr = (void*)((((uint32_t)&end / 4096) + 1) * 4096);
	kprintf("Kernel begin: 0x%x\tend: 0x%x\n", (uint32_t)baddr, (uint32_t)eaddr);
	for (uint32_t i = 0; i < (uint32_t)eaddr; i += 4096)
	{
		pmm_setbit(i/4096, PMM_MEMUSED);
	}
	
	/* Set the first bit to used, as this references null. We never want to be
	 * using null as memory as it is reserved to represent an error status.
	 */
	pmm_setbit(0, PMM_MEMUSED);
}

// This function assumes that pmm_init has been called; mmap and mmaplen should
// have been set appropriately by pmm_init
void pmm_setbit(uint32_t absbit, uint8_t value)
{
	uint32_t byte = absbit / 8;
	uint8_t bit = absbit % 8;
	uint8_t mask = (1 << bit);
	if (value)
		mmap[byte] |= mask;
	else
		mmap[byte] &= (~mask);
}

// Same assumptions as pmm_setbit
uint8_t pmm_getbit(uint32_t absbit)
{
	uint8_t retval;
	uint32_t byte = absbit / 8;
	uint8_t bit = absbit % 8;
	uint8_t mask = (1 << bit);
	retval = mmap[byte] & mask;
	if (retval)
		return 1;
	return 0;
}

uint32_t pmm_allocpage()
{
	/* We'll do an initial pass to see how many frames we can skip by testing
	 * for bytes equal to 0 sequentially
	 */
	uint32_t byte = 0;
	while (byte < mmaplen)
	{
		if (mmap[byte] == 0)
		{
			byte++;
			continue;
		}
		for (uint8_t i = 0; i < 8; i++)
		{
			if (pmm_getbit(byte * 8 + i))
			{
				pmm_setbit(byte * 8 + i, PMM_MEMUSED);
				return (byte * 8 + i) * 4096;
			}
		}
		byte++;
	}
	
	// If we reach this point then.... oops.
	return 0;
}

void pmm_freepage(uint32_t addr)
{
	uint32_t bit = addr / 4096;
	ASSERT(pmm_getbit(bit) == PMM_MEMUSED);
	pmm_setbit(bit, PMM_MEMFREE);
}

// This function should be called from vmm_init; so the pmm can map all required
// memory into the kernel address space
void pmm_postinit()
{
	for (uint32_t i = 0; i < mmaplen; i+=4096)
	{
		void* addr = (void*)(mmap + i);
		vmm_set_map(vmm_kernelpd(), addr, addr, PD_RW | PD_KERNEL);
	}
}
