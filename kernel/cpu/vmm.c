#include "vmm.h"
#include "pmm.h"
#include "common.h"
#include "isr.h"

// TODO: Revert changes making all memory usermode available

// Important note:
// All page directories and tables should be identity mapped within the kernel's
// address space, so that they can easily be modified by the kernel when needed

// Defined by the linker so we can find the end of kernel memory
extern uint32_t end;

// Required paging constructs
uint32_t* kernel_pd;

// Forward declare the handler
void pf_handler(registers_t* regs);

void vmm_init()
{
	// First let's allocate some memory for our kernel page directory
	kernel_pd = (uint32_t*)pmm_allocpage();
	
	// Now let's calculate how many page table entries we'll need to identity
	// map from the beginning of memory to the end of the kernel
	void* eaddr = (void*)((((uint32_t)&end / 4096) + 1) * 4096);
	uint32_t entries = (uint32_t)(eaddr) / 4096;
	
	// Calculate the number of page tables the required PTE's span
	uint32_t tables = entries / 1024;
	if (entries % 1024)
		tables++;
	
	kprintf("vmm_init entries: %i\ttables: %i\n", entries, tables);
	
	// Initially we need to clear the kernel page directory
	memset((uint8_t*)kernel_pd, 0, 4096);
	
	// Next we need to set up the kernel PD for the number of tables required
	// and allocate the memory for the tables
	// Settings:
	// PRESENT, RW, KERNEL
	uint32_t addr;
	for (uint32_t i = 0; i < tables; i++)
	{
		addr = pmm_allocpage();
		memset((uint8_t*)addr, 0, 4096);
		kernel_pd[i] =  addr;
		kernel_pd[i] |= PD_PRESENT;
		kernel_pd[i] |= PD_RW;
		kernel_pd[i] |= PD_KERNEL;
		//kernel_pd[i] |= PD_USERMODE;	// While we test user mode
	}
	
	// We should now fill in all the PTE's in the kernel page tables
	// Settings:
	// PRESENT, RW, KERNEL
	uint32_t* table;
	for (uint32_t i = 0; i < entries; i++)
	{
		table = (uint32_t*)(kernel_pd[i/1024] & PAGE_PADDR_MASK);
		table[i%1024] =  PAGE_GET_VADDR(i/1024, i%1024);
		table[i%1024] |= PT_PRESENT;
		table[i%1024] |= PT_RW;
		table[i%1024] |= PT_KERNEL;
		//table[i%1024] |= PT_USERMODE;
	}
	
	// Create tables for everything above 3GB (kernel heap)
	for (int i = 768; i < 1024; i++)
	{
		// Allocate the table space
		kernel_pd[i] = pmm_allocpage();
		
		// Clear the table
		memset((void*)kernel_pd[i], 0, 4096);
		
		// Apply the flags
		kernel_pd[i] |= PD_PRESENT;
		kernel_pd[i] |= PD_RW;
		kernel_pd[i] |= PD_KERNEL;
	}
	
	// Identity map the paging constructs (PD, PT's)
	vmm_set_map(kernel_pd, (void*)kernel_pd, (void*)kernel_pd, PD_RW | PD_KERNEL);
	for (int i = 0; i < 1024; i++)
	{
		if (kernel_pd[i] & PD_PRESENT)
			vmm_set_map(kernel_pd, (void*)(kernel_pd[i] & PAGE_PADDR_MASK), (void*)(kernel_pd[i] & PAGE_PADDR_MASK), PD_RW | /*PD_KERNEL*/ PD_USERMODE);
	}
	
	// Next we should set the current page directory
	set_page_directory(kernel_pd);
	
	// Ensure that the physical memory allocator is prepared for paging
	pmm_postinit();
	
	// Register the interrupt handler
	reg_int_handler(14, &pf_handler);
	
	// And last, enable paging!
	enable_paging();
}

void vmm_set_map(uint32_t* pd, void* vaddr, void* paddr, uint32_t flags)
{
	// Calculate which table and entry we need to be modifying
	uint32_t entry = (uint32_t)vaddr / 4096;
	uint32_t table = entry / 1024;
			 entry = entry % 1024;
	//kprintf("vmm_set_map table: %i\tentry: %i vaddr:0x%x paddr:0x%x\n", table, entry, vaddr, paddr);
	ASSERT(PAGE_GET_VADDR(table, entry) == (uint32_t)vaddr);
	
	// First ensure the page table has been created. If it has not, then create
	// it first.
	uint32_t addr;
	uint32_t* ptr;
	if (!(pd[table] & PD_PRESENT))
	{
		// Allocate space for the page table
		addr = pmm_allocpage();
		// Set up the flags for the page table
		pd[table] =  addr;
		pd[table] |= PD_PRESENT;
		pd[table] |= PD_RW;
		pd[table] |= PD_KERNEL;
		//pd[table] |= PD_USERMODE;
		pd[table] |= (flags & (~PD_REPLACE));
		// Map the table into the kernel's address space and the current address space
		vmm_set_map(kernel_pd, (void*)addr, (void*)addr, PD_RW | PD_KERNEL);
		vmm_set_map(vmm_currentpd(), (void*)addr, (void*)addr, PD_RW | PD_KERNEL | PD_REPLACE);
		
		// Clear the table
		memset((uint8_t*)addr, 0, 4096);
	}
	
	ptr = (uint32_t*)(pd[table] & PAGE_PADDR_MASK);
	
	// Next ensure the entry hasn't been reserved, unless otherwise specified in
	// the flags
	if (!(flags & PD_REPLACE))
		ASSERT((ptr[entry] & PT_PRESENT) == 0);
	flags &= (~PD_REPLACE);
	
	// Now set up the entry
	ptr[entry]  = (uint32_t)paddr & PAGE_PADDR_MASK;
	ptr[entry] |= flags | PT_PRESENT;
	
	// We've modified paging structures, so reload CR3
	uint32_t pd_addr;
	asm volatile("mov %%cr3, %0" : "=r" (pd_addr));
	asm volatile("mov %0, %%cr3" : : "r" (pd_addr));
}

uint32_t* vmm_kernelpd()
{
	return kernel_pd;
}

// This will copy the kernel page directory to be used in creating a new process
// It is up to the caller to map new pages as necessary
uint32_t* vmm_newpd()
{
	uint8_t* newpd = (uint8_t*)pmm_allocpage();
	vmm_set_map(kernel_pd, (void*)newpd, (void*)newpd, PD_RW | PD_KERNEL);
	vmm_set_map(vmm_currentpd(), (void*)newpd, (void*)newpd, PD_RW | PD_KERNEL | PD_REPLACE);
	uint8_t* kpd = (uint8_t*)kernel_pd;
	memcpy(newpd, kpd, 4096);
	uint32_t* pd = (uint32_t*)newpd;
	
	// Go through the PDE and clone all tables that are present below 3GB
	for (int i = 0; i < 768; i++)
	{
		if (pd[i] & PD_PRESENT)
		{
			// Find old page table physical address
			void* old = (void*)(pd[i] & PAGE_PADDR_MASK);
			// Allocate space for new page table
			void* new = (void*)(pmm_allocpage());
			// Map the new table in the kernel address space and the current one
			vmm_set_map(kernel_pd, new, new, PD_RW | PD_KERNEL);
			vmm_set_map(vmm_currentpd(), new, new, PD_RW | PD_KERNEL | PD_REPLACE);
			// Clone the page table
			memcpy((uint8_t*)new, (uint8_t*)old, 0x1000);
			// Mask out the page table physical address
			pd[i] &= PAGE_FLAGS_MASK;
			// Mask in the new page table physical address
			pd[i] |= (uint32_t)new;
			// Map the new table in itself
			vmm_set_map(pd, new, new, PD_RW | PD_KERNEL | PD_REPLACE);
		}
	}
	
	// Map the page directory into itself
	vmm_set_map(pd, (void*)pd, (void*)pd, PD_RW | PD_KERNEL | PD_REPLACE);
	
	return (uint32_t*)newpd;
}

uint32_t* vmm_currentpd()
{
	uint32_t pd_addr;
	asm volatile("mov %%cr3, %0" : "=r" (pd_addr));
	return (uint32_t*)pd_addr;
}

void pf_handler(registers_t* regs)
{
	// Get the address that called the fault
	uint32_t fault_addr;
	asm volatile("mov %%cr2, %0" : "=r"(fault_addr));
	kprintf("Page fault at 0x%x\n", fault_addr);
	
	// Determine what the fault was and output it
	if (regs->err_code & 0x1) kprintf("present\n"); else kprintf("not present\n");
	if (regs->err_code & 0x2) kprintf("writing\n"); else kprintf("reading\n");
	if (regs->err_code & 0x4) kprintf("user mode\n"); else kprintf("kernel mode\n");
	if (regs->err_code & 0x8) kprintf("reserved\n");
	if (regs->err_code & 0x10) kprintf("fetching instruction\n");
	
	kprintf("Instruction address: 0x%x\n", regs->eip);
	
	PANIC("Unhandled page fault");
}
