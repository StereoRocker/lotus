#ifndef MODULE_H
#define MODULE_H

#include <stdint.h>
#include "multiboot.h"

typedef struct {
	uint32_t* mod_start;
	uint32_t mod_length;
} mod_info_t;

// Marks the physical address spaces of the modules as used
void mod_initpmm(multiboot_t* multiboot);

// Pages the modules into memory, and allocates objects on the heap to keep track of them more easily
void mod_initvmm(multiboot_t* multiboot);

// Returns the number of modules
uint32_t mod_count();

// Returns the starting address of the module at index
uint32_t* mod_addr(uint32_t index);

// Returns the length of the module at index
uint32_t mod_len(uint32_t index);

#endif