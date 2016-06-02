#include "heap.h"
#include "vmm.h"
#include "pmm.h"
#include "common.h"

// This implements a growing kernel heap starting at the 3GB virtual mark
// NOTE: This heap never shrinks in size and will expand as large as necessary
//       (Max size is 1GB due to address space limits)
//       If the kernel heap takes up 1GB, run away and start again.

void* heap_start;
uint32_t heap_size;
header_t* heap_header;
footer_t* heap_footer;

// Initialises the heap at 3GB in the address space, with 4KiB space
void heap_init()
{
	// Initialise the heap constants
	heap_start = (void*)0xC0000000;
	heap_size = 0x1000;
	
	// Allocate memory for the heap and page it in
	void* page = (void*)pmm_allocpage();
	vmm_set_map(vmm_kernelpd(), heap_start, page, PD_RW | PD_KERNEL);
	
	// Set up the memory in the heap
	heap_header = (header_t*)heap_start;
	heap_header->magic = 0xCAFEBABE;
	heap_header->is_hole = 1;
	heap_header->size = heap_size;
	
	heap_footer = (footer_t*)((uint32_t)heap_start + heap_size - sizeof(footer_t));
	heap_footer->magic = 0xCAFEBABE;
	heap_footer->header = heap_header;
	
	heap_header->footer = heap_footer;
}

header_t* heap_condense(header_t* middle)
{
	header_t *left, *right;
	footer_t *leftf, *rightf, *middlef;
	
	ASSERT(middle->magic == 0xCAFEBABE);
	middlef = (footer_t*)(middle->footer);
	ASSERT(middle->magic == 0xCAFEBABE);
	
	leftf = (footer_t*)((uint32_t)middle - sizeof(footer_t));
	//kprintf("heap_condense: leftf 0x%x\n", leftf);
	if ((uint32_t)leftf > (uint32_t)heap_start)
	{
		ASSERT(leftf->magic == 0xCAFEBABE);
		left = leftf->header;
		ASSERT(left->magic == 0xCAFEBABE);
		
		// First, see if we can expand left
		if (left->is_hole)
		{
			// We can, so let's do it!
			left->footer = middlef;
			left->size = (uint32_t)((uint32_t)middlef + sizeof(footer_t) - (uint32_t)left);
			middlef->header = left;
			middle = left;
		}
	}
	
	right = (header_t*)((uint32_t)middlef + sizeof(footer_t));
	//kprintf("heap_condense: right 0x%x\n", right);
	if ((uint32_t)right < (uint32_t)heap_start + heap_size)
	{
		ASSERT(right->magic == 0xCAFEBABE);
		rightf = right->footer;
		ASSERT(rightf->magic == 0xCAFEBABE);
	
		// Next, see if we can expand right
		if (right->is_hole)
		{
			// We can, so let's do it!
			middle->footer = rightf;
			middle->size = (uint32_t)((uint32_t)rightf + sizeof(footer_t) - (uint32_t)middle);
			rightf->header = middle;
		}
	}
	
	// Now return the address of the new block
	return middle;
}

void* kmalloc(uint32_t size)
{
	// First find the smallest block
	header_t* header = heap_header;
	footer_t* footer;
	header_t* smallest = (header_t*)0;
	uint32_t reqsize = size + sizeof(header_t) + sizeof(footer_t);
	do
	{
		// Perform some assertions and calculations
		ASSERT(header->magic == 0xCAFEBABE);
		footer = header->footer;
		ASSERT(footer->magic == 0xCAFEBABE);
	
		// Check to see if the current block is free
		// If it is, consider it as a candidate to be used/split
		// Otherwise skip to the next header (if it exists)
		if (header->is_hole && header->size >= reqsize)
		{
			// Consider this as a candidate to be used/split
			// If smallest is null, set it to this
			if(smallest == (header_t*)0)
				smallest = header;
			else
			{
				// Otherwise see if this hole is smaller, and big enough 
				// If so, set smallest to this hole
				if (header->size < smallest->size)
					smallest = header;
			}
		}
		// Skip to the next header (straight after the current footer)
		header = (header_t*)((uint32_t)footer+sizeof(footer_t));
	}
	while (footer != heap_footer);
	
	// Output some debug information
	//kprintf("smallest: 0x%x\nsize: %i\nreq: %i\n", smallest, smallest->size, reqsize);
	
	/* Now consider what we have to do next, it will be one of 4 choices
	 * 1 - The block will be the exact size needed: we allocate it and return the address of the free memory
	 * 2 - The block will be larger than needed, but not large enough to hold another block: we allocate it and return the address of the free memory
	 * 3 - The block will be larger than needed, and large enough to split it into two blocks: we split it, allocate the correct size chunk
	  																						   Then test the block to the right of the extra block. If it is free, combine it with the extra block.
	 * 4 - No suitable block was found: we allocate more memory to the heap (enough to create the new block)
	                                    Then test the block to the left of the new block, if it is free, combine it with the new block
	 */
	
	if (smallest == (header_t*)0)	// Condition 4
	{
		//kprintf("kmalloc: condition 4\n");
		// Calculate how many more pages are required
		uint32_t pages = reqsize / 4096;
		if (reqsize % 4096) pages++;
		
		// Map the pages in
		for (uint32_t i = 0; i < pages; i++)
		{
			void *v, *p;
			p = (void*)pmm_allocpage();
			v = (void*)((uint32_t)heap_start + heap_size + (i*4096));
			if (p == (void*)0)
				PANIC("kmalloc: out of memory");
			vmm_set_map(vmm_kernelpd(), v, p, PD_RW | PD_KERNEL);
		}
		
		// Calculate the new heap size
		heap_size += (pages * 4096);
		//kprintf("kmalloc: heap_size %i\n", heap_size);
		
		// Create the new block
		header = (header_t*)((uint32_t)heap_footer + sizeof(footer_t));
		header->magic = 0xCAFEBABE;
		header->size = pages * 4096;
		
		footer = (footer_t*)((uint32_t)heap_start + heap_size - sizeof(footer_t));
		footer->magic = 0xCAFEBABE;
		footer->header = header;
		
		header->footer = footer;
		
		heap_footer = footer;
		
		// Condense blocks
		heap_condense(header);
		//kprintf("kmalloc: condition 4, condense complete\n");
		
		// Recursively call kmalloc to allocate the memory (knowing there is enough space)
		return kmalloc(size);
		
	} else if (smallest->size == reqsize)	// Condition 1
	{
		//kprintf("kmalloc: condition 1\n");
		// Allocate the block
		smallest->is_hole = 0;
		
		// Return the address of free memory
		return (void*)((uint32_t)smallest+sizeof(header_t));
	} else if (smallest->size > reqsize && smallest->size < reqsize + sizeof(header_t) + sizeof(footer_t) + 1)	// Condition 2
	{
		//kprintf("kmalloc: condition 2\n");
		// Allocate the block
		smallest->is_hole = 0;
		
		// Return the address of free memory
		return (void*)((uint32_t)smallest+sizeof(header_t));
	} else {								// Condition 3
		//kprintf("kmalloc: condition 3\n");
		
		// Split the block
		footer_t *newf, *oldf;
		header_t *new;
		
		oldf = (footer_t*)smallest->footer;
		//kprintf("kmalloc: oldf 0x%x\n", oldf);
		
		// Create the new footer for this block
		smallest->size = reqsize;
		newf = (footer_t*)((uint32_t)smallest+reqsize-sizeof(footer_t));
		//kprintf("kmalloc: newf 0x%x\n", newf);
		newf->magic = 0xCAFEBABE;
		newf->header = smallest;
		smallest->footer = newf;
		
		// Allocate the block
		smallest->is_hole = 0;
		
		// Create the new header for the extra block
		new = (header_t*)((uint32_t)newf + sizeof(footer_t));
		//kprintf("kmalloc: new 0x%x\n", new);
		new->magic = 0xCAFEBABE;
		new->size = (uint32_t)((uint32_t)(oldf)+sizeof(footer_t)-(uint32_t)new);
		new->is_hole = 1;
		new->footer = oldf;
		ASSERT(new->magic == 0xCAFEBABE);
		
		oldf->header = new;
		
		// Condense the new free block if possible
		heap_condense(new);
		
		// Return the address of free memory
		return (void*)((uint32_t)smallest+sizeof(header_t));
	}
	
	PANIC("kmalloc: Undefined condition occured");
	return (void*)0;
}

void kfree(void* mem)
{
	header_t* header = (header_t*)((uint32_t)mem - sizeof(header_t));
	ASSERT(header->magic == 0xCAFEBABE);
	footer_t* footer = (footer_t*)header->footer;
	ASSERT(footer->magic == 0xCAFEBABE);
	
	// Mark the header as unallocated
	header->is_hole = 1;
	
	// Attempt to condense the memory
	heap_condense(header);
}
