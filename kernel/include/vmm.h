#ifndef VMM_H
#define VMM_H
#include <stdint.h>

/* Externally accessible functions */
void vmm_init();
void vmm_set_map(uint32_t* pd, void* vaddr, void* paddr, uint32_t flags);
uint32_t* vmm_kernelpd();
uint32_t* vmm_newpd();
uint32_t* vmm_currentpd();

/* Page directory entry definitions */
#define PD_PRESENT 	0x00000001
#define PD_ABSENT	0

#define PD_RW 		0x00000002
#define PD_RO		0

#define PD_KERNEL	0
#define PD_USERMODE	0x00000004

#define PD_WTHROUGH	0x00000008
#define PD_WBACK	0

#define PD_CACHEON	0
#define PD_CACHEOFF	0x00000010

#define PD_ACCESSED	0x00000020

#define PD_REPLACE  0x10000000

/* Page table entry definitions */
#define PT_PRESENT	0x00000001
#define PT_ABSENT 	0

#define PT_RW		0x00000002
#define PT_RO		0

#define PT_KERNEL	0
#define PT_USERMODE	0x00000004

#define PT_WTHROUGH	0x00000008
#define PT_WBACK	0

#define PT_CACHEON	0
#define PT_CACHEOFF	0x00000010

#define PT_ACCESSED	0x00000020
#define PT_DIRTY 	0x00000040

/* Assembler functions */
void enable_paging();
void set_page_directory(uint32_t* page_directory);

/* Useful constants and macros */
#define PAGE_GET_VADDR(table, entry) ((table * 1024 * 0x1000) + (entry * 0x1000))
#define PAGE_PADDR_MASK 0xFFFFF000
#define PAGE_FLAGS_MASK 0x00000FFF

#endif
