#ifndef PMM_H
#define PMM_H
#include <stdint.h>

#define PMM_MEMFREE 1
#define PMM_MEMUSED 0

void pmm_init(uint32_t len, uint32_t addr);
void pmm_setbit(uint32_t absbit, uint8_t value);
uint8_t pmm_getbit(uint32_t absbit);
uint32_t pmm_allocpage();
void pmm_freepage(uint32_t addr);
void pmm_postinit();

#endif
