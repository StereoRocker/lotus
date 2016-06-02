#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

void timer_init(uint32_t freq);
uint64_t timer_getticks();

#endif
