#ifndef HEAP_H
#define HEAP_H

#include <stdint.h>

struct footer_t;

typedef struct __attribute__ ((__packed__)) header_t
{
	uint32_t magic;		// Magic number, used for error checking
	uint8_t is_hole;	// 1 if this is a hole, 0 if this is a block
	void* footer;	// Pointer to the block footer
	uint32_t size;		// Size of the block, including this and the footer
} header_t;

typedef struct __attribute__ ((__packed__)) footer_t
{
	uint32_t magic;		// Magic number, same as in header_t
	void* header;	// Pointer to the block header
} footer_t;

void heap_init();
void* kmalloc();
void kfree(void* mem);

#endif
