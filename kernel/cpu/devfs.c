#include "common.h"
#include "heap.h"
#include "task.h"
#include "devfs.h"
#include "syscall.h"
#include "mutex.h"
#include "lotus/syscall.h"
#include "lotus/devfs.h"

dev_t* devices;
mutex devmutex;

void devfs_reg(char* name)
{
	if (name == 0)
		return;
	if (strlen(name) >= DEVNAME_MAX)
		return;
	
	// Create the device
	dev_t* dev = kmalloc(sizeof(dev_t));
	memset((uint8_t*)dev, 0, sizeof(dev_t));
	strcpy(dev->name, name);
	dev->pid = getpid();
	dev->next = (struct dev_t*)0;
	
	// Lock the mutex
	mutex_lock(&devmutex);
	
	// If the list doesn't exist yet, use this as the starting point
	if (devices == (dev_t*)0)
	{
		devices = dev;
		mutex_unlock(&devmutex);
		return;
	}
	
	// Find the end of the list
	dev_t* d = devices;
	while (d->next != (struct dev_t*)0)
		d = (dev_t*)d->next;
	
	// Add this device to the end of the list
	d->next = (struct dev_t*)dev;
	
	// Unlock the mutex
	mutex_unlock(&devices);
}

uint32_t devfs_reg_sys(uint32_t data)
{
	// devfs_reg checks it's parameters are sane
	devfs_reg((char*)data);
	
	return 0;
}

void devfs_init()
{
	devices = (dev_t*)devices;
	devmutex = 0;
	
	syscall_register(devfs_reg_sys, SYS_DEVFSREG);
}