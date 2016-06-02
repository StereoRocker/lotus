#ifndef DT_H
#define DT_H

#include <stdint.h>

// Initialises the descriptor tables required by the CPU
void dt_init();
void set_kstack(uint32_t stack);

#endif
