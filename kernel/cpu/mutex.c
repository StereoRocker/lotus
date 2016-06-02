#include "mutex.h"
#include "task.h"

void mutex_lock(mutex* mut)
{
	uint8_t val = 1;
	while (val == 1)
	{
		asm volatile("lock; xchg %0, %1" : "+m" (mut), "=a"(val) : "1"(val) : "cc");
		asm volatile("int $0x20");
	}
}

void mutex_unlock(mutex* mut)
{
	*mut = 0;
}