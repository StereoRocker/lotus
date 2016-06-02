#ifndef LOTUS_IPC_H
#define LOTUS_IPC_H

// This file should be integrated into the system root as well, so apps can use it!
// Well, apps or a library to integrate Lotus IPC into an app. So maybe a driver as well.

typedef struct {
	uint32_t target_pid;
	uint32_t length;
	uint8_t* data;
} ipcsend_t;

#endif