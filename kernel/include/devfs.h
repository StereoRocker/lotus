#ifndef DEVFS_H
#define DEVFS_H

#include <stdint.h>
#include "lotus/devfs.h"

typedef struct {
	char name[DEVNAME_MAX];
	uint32_t pid;
	struct dev_t* next;
} dev_t;

void devfs_init();
uint32_t devfs_open(char* devname);
uint32_t devfs_close(char* devname);
uint32_t devfs_read(char* devname, uint64_t offset, uint32_t length, uint8_t* buf);
uint32_t devfs_write(char* devname, uint64_t offset, uint32_t length, uint8_t* buf);

#endif