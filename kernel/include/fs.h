#ifndef FS_H
#define FS_H

#include <stdint.h>
#include "lotus/vfs.h"

typedef struct {
	uint32_t pid;				// Process ID of this FS
	uint8_t fsID;				// Individual ID of the mounted FS (given to us by the driver)
	char path[4096];			// Path this FS is mounted on
	char dev[4096];				// Path of the device the FS is mounted on
	struct fsmount_t* next;		// Pointer to the next mounted FS in the list
} fsmount_t;

#define FSNAME_MAX 64
typedef struct {
	uint32_t pid;		// Process ID of this FS
	char	 name[64];	// Name of this FS
	struct fstype_t* next;
} fstype_t;

// Internal for kernel use only
void vfs_init();
uint32_t vfs_fscount();
void vfs_print();

// Handles processes registering as filesystems
uint32_t vfs_register_sys(uint32_t data);
void vfs_register(char* name);

// Handles mounting & unmounting a filesystem
uint32_t vfs_mount_sys(uint32_t data);
uint32_t vfs_unmount_sys(uint32_t data);
uint32_t vfs_mount(char* unpath, char* dev, char* fsname);
uint32_t vfs_unmount(char* path);

// Handles opening & closing a file
uint32_t vfs_open_sys(uint32_t data);
uint32_t vfs_close_sys(uint32_t data);
uint32_t vfs_open(char* path);
uint32_t vfs_close(char* path);

// Handles reading & writing a file
uint32_t vfs_read_sys(uint32_t data);
uint32_t vfs_write_sys(uint32_t data);
uint32_t vfs_read(char* path, uint64_t offset, uint32_t length, uint8_t* buf);
uint32_t vfs_write(char* path, uint64_t offset, uint32_t length, uint8_t* buf);

// Handles getting information about files & directories
uint32_t vfs_filesize_sys(uint32_t data);
uint32_t vfs_dirsize_sys(uint32_t data);
uint32_t vfs_dirlist_sys(uint32_t data);
uint32_t vfs_filesize(char* path);
uint32_t vfs_dirsize(char* path);
uint32_t vfs_dirlist(char* path, uint32_t maxents, dirent_t* dirents);

#endif