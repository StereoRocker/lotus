#include <stdint.h>
#include "fs.h"
#include "common.h"
#include "heap.h"
#include "task.h"
#include "mutex.h"
#include "syscall.h"			// This file is different to the one in lotus/
#include "devfs.h"				// This file is different to the one in lotus/
#include "lotus/syscall.h"
#include "lotus/vfs.h"

// This mutex is required as it's highly likely that more than 1 FS driver will
// be loaded at once. The init program will likely launch all drivers that are 
// available and desired
mutex fsmutex;
fstype_t *filesystems;

// This mutex is required as it's somewhat likely that multiple filesystems
// will be mounted at the same time.
mutex mountmutex;
fsmount_t *mounts;

void vfs_register(char* name)
{
	// Register the calling PID as a filesystem driver
	fstype_t *fs = kmalloc(sizeof(fstype_t));
	fs->pid = getpid();
	strncpy(fs->name, name, 50);
	fs->next = (struct fstype_t*)0;
	
	// Add this FS to the linked list
	
	// Lock the mutex
	mutex_lock(&fsmutex);
	
	// If this is the first FS, then make it the start of the linked list
	if (filesystems == 0)
	{
		filesystems = fs;
		mutex_unlock(&fsmutex);
		return;
	}
	
	// Find the end of the linked list and add this FS to it
	fstype_t *fsp = filesystems;
	while (fsp->next != 0)
		fsp = (fstype_t*)fsp->next;
	fsp->next = (struct fstype_t*)fs;
	
	// Unlock the mutex
	mutex_unlock(&fsmutex);
}

// This will be the syscall that handles processes calling to register as a filesystem
uint32_t vfs_register_sys(uint32_t data)
{
	char* name = (char*)data;
	if (strlen(name) > FSNAME_MAX)
		return 0;
	
	vfs_register(name);
	return 0;
}

uint32_t vfs_fscount()
{
	// Count the number of filesystems registered
	uint32_t count = 0;
	if (filesystems == 0)
		return 0;
	
	fstype_t* fsp = filesystems;
	while (fsp != 0)
	{
		count++;
		fsp = (fstype_t*)fsp->next;
	}
	return count;
}

uint32_t vfs_mount(char* path, char* dev, char* fsname)
{
	// Ensure a filesystem is registered
	if (filesystems == 0)
		return 0;
	
	// Ensure the passed parameters are sane
	if (path == 0 || dev == 0 || fsname == 0)
		return 0;
	
	if (strlen(path) >= PATH_MAX)
		return 0;
	
	if (strlen(dev) >= PATH_MAX)
		return 0;
	
	if (strlen(fsname) >= FSNAME_MAX)
		return 0;
	
	// First search for the named FS
	fstype_t *fsp = filesystems;
	while (fsp != 0)
	{
		if (strcmp(fsp->name, fsname) == 0)
			break;
		
		// This would mean the named FS wasn't found
		if (fsp->next == 0)
			return 0;
		
		// Look at the next FS
		fsp = (fstype_t*)fsp->next;
	}
	
	// The named FS has been found as is pointed to by fsp
	
	// Otherwise, send a mount request to the driver
	// Exception: if the PID is 0, mount devfs (more to details come, devfs is the only internal FS)
	fsreq_t* req = kmalloc(sizeof(fsreq_t) + PATH_MAX);
	char* pbuf = (char*)((uint32_t)req + sizeof(fsreq_t));
	// Copy the device name to the packet
	strncpy(pbuf, dev, 4096);
	// Fill in the other fields of the packet
	req->magic = FS_REQ_MAGIC;
	req->request = FS_MOUNT;
	req->fsID = 0;	// This is not a defined field for FS_MOUNT
	req->length = 4096;
	
	// Send the packet!
	ipc_send(fsp->pid, (uint8_t*)req, sizeof(fsreq_t) + PATH_MAX);
	
	// We're done with the request, so free it
	kfree(req);
	
	// Wait for the response
	while (1)
	{
		while (ipc_msglen() < 0)
			task_switch();
		
		// Get the reponse and interpret it
		fsres_t* res = kmalloc(ipc_msglen());
		if (res == 0)	// We couldn't allocate a space for the response
		{
			PANIC("vfs_mount: couldn't allocate res");
		}
		
		ipc_getdata((uint8_t*)res);
		
		// Check the magic number is correct
		if (res->magic != FS_RES_MAGIC)
		{
			ipc_nextmsg();	// Get the next message in the queue
			continue;		// Start the loop again
		}
		
		// Check if the response is success
		if (res->success == 0)
		{
			ipc_nextmsg();	// Remove this message from the queue
			return 0;		// Return error
		}
		
		// Ensure the length of the data after the response is 1
		ASSERT(res->length == 1);
		
		// If the response was success, add this mounting to the list
		uint8_t* fsID = (uint8_t*)((uint32_t)res + sizeof(fsres_t));
		fsmount_t* mount = (fsmount_t*)kmalloc(sizeof(fsmount_t));
		mount->pid = fsp->pid;
		mount->fsID = *fsID;
		strcpy(mount->path, path);
		strcpy(mount->dev, dev);
		mount->next = (struct fsmount_t*)0;
		
		// Lock the mutex
		mutex_lock(&mountmutex);
		
		// Add mount to the end of the mountpoint list
		if (mounts == 0)
		{
			mounts = mount;
			mutex_unlock(&mountmutex);
			return 1;
		}
		
		fsmount_t* m = mounts;
		while (m->next != 0)
			m = (fsmount_t*)m->next;
		
		m->next = (struct fsmount_t*)mount;
		
		// Unlock the mutex
		mutex_unlock(&mountmutex);
		
		// Tell the IPC system we want the next message
		ipc_nextmsg();
		return 1;
	}
}

uint32_t vfs_mount_sys(uint32_t data)
{
	// Parse the structure pointed to by data to get the arguments to pass to vfs_mount
	vm_sys_t* params = (vm_sys_t*)data;
	
	// vfs_mount ensures its parameters are sane
	return vfs_mount(params->path, params->dev, params->fsname);
}

void vfs_print()
{
	// Print out all mounted filesystems via kprintf
	if (mounts == 0)
	{
		kprintf("No mounted filesystems\n");
		return;
	}
	
	fsmount_t* m = mounts;
	while (m != 0)
	{
		kprintf("%s mounted on %s PID: %i, fsID: %i\n", m->path, m->dev, m->pid, m->fsID);
		m = (fsmount_t*)m->next;
	}
}

// This function takes 3 arguments:
// fullpath		The full path to parse and get information about
// mount		A pointer to the pointer to write the mounting information pointer to
// relativepath	A pointer to the pointer to write the relative path pointer to
// return		1 on success, 0 otherwise
uint32_t vfs_getfsinfo(char* fullpath, fsmount_t** mount, char** relativepath)
{
	// Check a mountpoint exists
	if (mounts == 0)
		return 0;
	
	// Loop through the mounts and find the correct mount for this FS
	fsmount_t* mp = 0;
	fsmount_t* tm;
	uint32_t len = 0;
	
	tm = mounts;
	while (tm != 0)
	{
		// Compare the paths. If the entire path of tm fits in fullpath, set mp to tm
		if (strcmp(tm->path, fullpath) == 0)
		{
			if (strlen(tm->path) > len)
			{
				mp = tm;
				len = strlen(tm->path);
			}
		}
		
		tm = (fsmount_t*)tm->next;
	}
	
	// If mp wasn't set, no suitable mountpoint was found
	if (mp == 0)
		return 0;
	
	// Write the mount info to mount
	*mount = mp;
	
	// Get the path
	
	// If the mountpoint path is / then the fullpath is already the relativepath
	if (strcmp(mp->path, "/") == 0)
	{
		*relativepath = fullpath;
	} else {
		// Otherwise, extract the relative path
		*relativepath = (char*)((uint32_t)fullpath + strlen(mp->path));
	}
	
	// Return success
	return 1;
}

// Return value indicates success
// 1 - success, otherwise 0
uint32_t vfs_open(char* fullpath)
{
	// First ensure fullpath is sane
	if (fullpath == 0)
		return 0;
	if (strlen(fullpath) >= PATH_MAX)
		return 0;
	
	// Then get information about the filesystem, and what the relative path to pass to the filesystem should be
	char* relpath;
	fsmount_t* mount;
	uint32_t ret = vfs_getfsinfo(fullpath, &mount, &relpath);
	
	if (ret == 0)
		return 0;
	
	// Now we test for PID = 0
	if (mount->pid == 0)
	{
		// Do devfs stuff
		
		
		// devfs isn't implemented, return failure
		return 0;
	}
	
	// We perform an IPC request to the correct FS driver
	// We need to pass the relative path in the info
	
	// Create the request packet
	fsreq_t* req = kmalloc(sizeof(fsreq_t) + PATH_MAX);
	memset((void*)req, 0, sizeof(fsreq_t) + PATH_MAX);
	req->magic = FS_REQ_MAGIC;
	req->request = FS_OPEN;
	req->fsID = mount->fsID;
	req->length = PATH_MAX;
	
	// Copy the relative path
	char* reqpath = (char*)((uint32_t)req + sizeof(fsreq_t));
	strcpy(reqpath, relpath);
	
	// Send the request
	ipc_send(mount->pid, (uint8_t*)req, sizeof(fsreq_t) + PATH_MAX);
	
	// Wait for the response
	while (1)
	{
		// Poll is a response has been received
		while (ipc_msglen() < 0)
			task_switch();
		
		// Get the reponse and interpret it
		fsres_t* res = kmalloc(ipc_msglen());
		if (res == 0)	// We couldn't allocate a space for the response
		{
			PANIC("vfs_open: couldn't allocate res");
		}
		
		ipc_getdata((uint8_t*)res);
		
		// Check the magic number is correct
		if (res->magic != FS_RES_MAGIC)
		{
			ipc_nextmsg();	// Get the next message in the queue
			continue;		// Start the loop again
		}
		
		// Check if the response is success
		if (res->success == 0)
		{
			ipc_nextmsg();	// Remove this message from the queue
			return 0;		// Return error
		}
		
		// Remove this message from the queue
		ipc_nextmsg();
		// Return success
		return 1;
	}
	
	return 0;
}

uint32_t vfs_open_sys(uint32_t data)
{
	// vfs_open ensures it's parameters are sane
	return vfs_open((char*)data);
}

// Return value indicates success
// 1 - success, otherwise 0
uint32_t vfs_close(char* fullpath)
{
	// First ensure fullpath is sane
	if (fullpath == 0)
		return 0;
	if (strlen(fullpath) >= PATH_MAX)
		return 0;
	
	// Then get information about the filesystem, and what the relative path to pass to the filesystem should be
	char* relpath;
	fsmount_t* mount;
	uint32_t ret = vfs_getfsinfo(fullpath, &mount, &relpath);
	
	if (ret == 0)
		return 0;
	
	// Now we test for PID = 0
	if (mount->pid == 0)
	{
		// Do devfs stuff
		
		
		// devfs isn't implemented, return failure
		return 0;
	}
	
	// We perform an IPC request to the correct FS driver
	// We need to pass the relative path in the info
	
	// Create the request packet
	fsreq_t* req = kmalloc(sizeof(fsreq_t) + PATH_MAX);
	memset((void*)req, 0, sizeof(fsreq_t) + PATH_MAX);
	req->magic = FS_REQ_MAGIC;
	req->request = FS_CLOSE;
	req->fsID = mount->fsID;
	req->length = PATH_MAX;
	
	// Copy the relative path
	char* reqpath = (char*)((uint32_t)req + sizeof(fsreq_t));
	strcpy(reqpath, relpath);
	
	// Send the request
	ipc_send(mount->pid, (uint8_t*)req, sizeof(fsreq_t) + PATH_MAX);
	
	// Wait for the response
	while (1)
	{
		// Poll is a response has been received
		while (ipc_msglen() < 0)
			task_switch();
		
		// Get the reponse and interpret it
		fsres_t* res = kmalloc(ipc_msglen());
		if (res == 0)	// We couldn't allocate a space for the response
		{
			PANIC("vfs_close: couldn't allocate res");
		}
		
		ipc_getdata((uint8_t*)res);
		
		// Check the magic number is correct
		if (res->magic != FS_RES_MAGIC)
		{
			ipc_nextmsg();	// Get the next message in the queue
			continue;		// Start the loop again
		}
		
		// Check if the response is success
		if (res->success == 0)
		{
			ipc_nextmsg();	// Remove this message from the queue
			return 0;		// Return error
		}
		
		// Remove this message from the queue
		ipc_nextmsg();
		// Return success
		return 1;
	}
	
	return 0;
}

uint32_t vfs_close_sys(uint32_t data)
{
	// vfs_close ensures it's parameters are sane
	return vfs_close((char*)data);
}

// Returns the number of bytes that were read into buf
uint32_t vfs_read(char* path, uint64_t offset, uint32_t length, uint8_t* buf)
{
	// Ensure the parameters are sane
	if (path == 0)
		return 0;
	if (buf == 0)
		return 0;
	if (strlen(path) >= PATH_MAX)
		return 0;
	if (length == 0)
		return 0;
	
	// Get the file system info
	char* relpath;
	fsmount_t* mount;
	uint32_t ret = vfs_getfsinfo(path, &mount, &relpath);
	
	if (ret == 0)
		return 0;
	
	// Now we test for PID = 0
	if (mount->pid == 0)
	{
		// Do devfs stuff
		
		
		// devfs isn't implemented, return failure
		return 0;
	}
	
	// We perform an IPC request to the correct FS driver
	// We need to pass the relative path in the info
	
	// Create the request packet
	#define FSREAD_REQSIZE (sizeof(fsreq_t) + PATH_MAX + sizeof(uint64_t) + sizeof(uint32_t))
	fsreq_t* req = kmalloc(FSREAD_REQSIZE);
	memset((void*)req, 0, FSREAD_REQSIZE);
	req->magic = FS_REQ_MAGIC;
	req->request = FS_READ;
	req->fsID = mount->fsID;
	req->length = FSREAD_REQSIZE - sizeof(fsreq_t);
	
	// Set up the parameter pointers
	char* reqpath = (char*)((uint32_t)req + sizeof(fsreq_t));
	uint64_t* reqoff = (uint64_t*)((uint32_t)reqpath + PATH_MAX);
	uint32_t* reqlen = (uint32_t*)((uint32_t)reqoff + sizeof(uint64_t));
	
	// Copy the relative path
	strcpy(reqpath, relpath);
	*reqoff = offset;
	*reqlen = length;
	
	// Send the packet
	ipc_send(mount->pid, (uint8_t*)req, sizeof(fsreq_t) + PATH_MAX);
	
	// Wait for the response
	while (1)
	{
		// Poll is a response has been received
		while (ipc_msglen() < 0)
			task_switch();
		
		// Get the reponse and interpret it
		fsres_t* res = kmalloc(ipc_msglen());
		if (res == 0)	// We couldn't allocate a space for the response
		{
			PANIC("vfs_read: couldn't allocate res");
		}
		
		ipc_getdata((uint8_t*)res);
		
		// Check the magic number is correct
		if (res->magic != FS_RES_MAGIC)
		{
			ipc_nextmsg();	// Get the next message in the queue
			continue;		// Start the loop again
		}
		
		// Check if the response is success
		if (res->success == 0)
		{
			ipc_nextmsg();	// Remove this message from the queue
			return 0;		// Return error
		}
		
		// Get the pointer to the data
		uint8_t* data = (uint8_t*)((uint32_t)res + sizeof(fsres_t));
		
		// Get the smaller of the length of the response and the requested length
		uint32_t smalllen = res->length;
		if (smalllen > length)
			smalllen = length;
		
		// Copy the data
		memcpy(buf, data, smalllen);
		
		// Remove this message from the queue
		ipc_nextmsg();
		// Return success
		return smalllen;
	}
}

uint32_t vfs_read_sys(uint32_t data)
{
	vr_sys_t* params = (vr_sys_t*)data;
	
	// vfs_read ensures it's parameters are sane
	return vfs_read(params->path, params->offset, params->length, params->buf);
}

// Returns the number of bytes that were read into buf
uint32_t vfs_write(char* path, uint64_t offset, uint32_t length, uint8_t* buf)
{
	// Ensure the parameters are sane
	if (path == 0)
		return 0;
	if (buf == 0)
		return 0;
	if (strlen(path) >= PATH_MAX)
		return 0;
	if (length == 0)
		return 0;
	
	// Get the file system info
	char* relpath;
	fsmount_t* mount;
	uint32_t ret = vfs_getfsinfo(path, &mount, &relpath);
	
	if (ret == 0)
		return 0;
	
	// Now we test for PID = 0
	if (mount->pid == 0)
	{
		// Do devfs stuff
		
		
		// devfs isn't implemented, return failure
		return 0;
	}
	
	// We perform an IPC request to the correct FS driver
	// We need to pass the relative path in the info
	
	// Create the request packet
	#define FSREAD_REQSIZE (sizeof(fsreq_t) + PATH_MAX + sizeof(uint64_t) + sizeof(uint32_t))
	fsreq_t* req = kmalloc(FSREAD_REQSIZE);
	memset((void*)req, 0, FSREAD_REQSIZE);
	req->magic = FS_REQ_MAGIC;
	req->request = FS_READ;
	req->fsID = mount->fsID;
	req->length = FSREAD_REQSIZE - sizeof(fsreq_t);
	
	// Set up the parameter pointers
	char* reqpath = (char*)((uint32_t)req + sizeof(fsreq_t));
	uint64_t* reqoff = (uint64_t*)((uint32_t)reqpath + PATH_MAX);
	uint32_t* reqlen = (uint32_t*)((uint32_t)reqoff + sizeof(uint64_t));
	
	// Copy the relative path
	strcpy(reqpath, relpath);
	*reqoff = offset;
	*reqlen = length;
	
	// Send the packet
	ipc_send(mount->pid, (uint8_t*)req, sizeof(fsreq_t) + PATH_MAX);
	
	// Wait for the response
	while (1)
	{
		// Poll is a response has been received
		while (ipc_msglen() < 0)
			task_switch();
		
		// Get the reponse and interpret it
		fsres_t* res = kmalloc(ipc_msglen());
		if (res == 0)	// We couldn't allocate a space for the response
		{
			PANIC("vfs_read: couldn't allocate res");
		}
		
		ipc_getdata((uint8_t*)res);
		
		// Check the magic number is correct
		if (res->magic != FS_RES_MAGIC)
		{
			ipc_nextmsg();	// Get the next message in the queue
			continue;		// Start the loop again
		}
		
		// Check if the response is success
		if (res->success == 0)
		{
			ipc_nextmsg();	// Remove this message from the queue
			return 0;		// Return error
		}
		
		// Get the pointer to the data
		uint8_t* data = (uint8_t*)((uint32_t)res + sizeof(fsres_t));
		
		// Get the smaller of the length of the response and the requested length
		uint32_t smalllen = res->length;
		if (smalllen > length)
			smalllen = length;
		
		// Copy the data
		memcpy(buf, data, smalllen);
		
		// Remove this message from the queue
		ipc_nextmsg();
		// Return success
		return smalllen;
	}
}

void vfs_init()
{
	filesystems = (fstype_t*)0;
	fsmutex = 0;
	
	// Set up a dummy mounting on /dev w/PID 0
	// PID 0 will tell the VFS that we should redirect the request to a registered device
	
	// We should probably call devfs_init() here. Not decided whether it should be called here or in kmain
	devfs_init();
	
	fsmount_t* mdev = kmalloc(sizeof(fsmount_t));
	memset((void*)mdev, 0, sizeof(fsmount_t));
	strcpy(mdev->path, "/dev");
	strcpy(mdev->dev, "none");
	mdev->pid = 0;
	mdev->fsID = 0;
	mdev->next = (struct fsmount_t*)0;
	
	mounts = (fsmount_t*)mdev;
	mountmutex = 0;
	
	// Register system calls
	syscall_register(vfs_register_sys, SYS_VFSREG);
	syscall_register(vfs_open_sys, SYS_VFSOPEN);
	syscall_register(vfs_close_sys, SYS_VFSCLOSE);
	syscall_register(vfs_read_sys, SYS_VFSREAD);
	//syscall_register(vfs_write_sys, SYS_VFSWRITE);
	syscall_register(vfs_mount_sys, SYS_VFSMOUNT);
	//syscall_register(vfs_unmount_sys, SYS_VFSUNMOUNT);
	//syscall_register(vfs_filesize_sys, SYS_VFSFILESIZE);
	//syscall_register(vfs_dirsize_sys, SYS_VFSDIRSIZE);
	//syscall_register(vfs_dirlist_sys, SYS_VFSDIRLIST);
}