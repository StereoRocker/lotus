#ifndef LOTUS_VFS_H
#define LOTUS_VFS_H

// This file is available to user mode applications as well!

/* IPC packet specifics */

// Magic field definitions
#define FS_REQ_MAGIC 0xF5F5CAFE
#define FS_RES_MAGIC 0xF5F5BABE

// Request field definitions
#define FS_OPEN 	1
#define FS_CLOSE 	2
#define FS_READ		3
#define FS_WRITE	4
#define FS_MOUNT	5
#define FS_UNMOUNT	6
#define FS_FILESIZE	7

// Unlike other implementations, the hard maximum path size is 4096 bytes
// including null terminator. The VFS structures don't allow for a path to be
// longer than this
#define PATH_MAX 4096

typedef struct {
	uint32_t 	magic;		// Should be 0xF5F5CAFE
	uint8_t 	request;	// Definitions above
	uint8_t 	fsID;		// The ID identifying which specific mount point to reference
	uint32_t 	length;		// The length of the data to follow (depends on the request)
	// Data follows this structure in the IPC packet. Have fun with pointers!
} fsreq_t;

typedef struct {
	uint32_t magic;		// Should be 0xF5F5BABE
	uint8_t success;	// Determines whether the operation was successful or not
	uint32_t length;	// The length of the data to follow (depends on the request)
	// Data follows this structure in the IPC packet. Have fun with pointers!
} fsres_t;

typedef struct {
	char	name[4096];
	uint8_t	isDir;
} dirent_t;

/* Syscall specifics */

// Structure that the vfs_mount syscall should be passed
typedef struct {
	char* path;
	char* dev;
	char* fsname;
} vm_sys_t;

// Structure that the vfs_read syscall should be passed
typedef struct {
	char* path;
	uint64_t offset;
	uint32_t length;
	uint8_t* buf;
} vr_sys_t;

#endif