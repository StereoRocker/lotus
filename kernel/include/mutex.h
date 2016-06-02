#ifndef MUTEX_H
#define MUTEX_H

#include <stdint.h>

typedef uint8_t mutex;
void mutex_lock(mutex* mut);	// Locks the mutex mut, will wait until it's locked
void mutex_unlock(mutex* mut);	// Unlocks the mutex mut

#endif