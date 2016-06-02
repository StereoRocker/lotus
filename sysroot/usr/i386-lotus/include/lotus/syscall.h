#ifndef LOTUS_SYSCALL_H
#define LOTUS_SYSCALL_H

// This file is available to user mode applications as well!

/* Tasking syscalls */
#define SYS_TASKSBRK		 0
#define SYS_TASKFORK		 1
#define SYS_TASKEXIT		 2
#define SYS_TASKYIELD		 3

/* IPC syscalls */
#define SYS_IPCSEND 		 4
#define SYS_IPCMSGLEN 		 5
#define SYS_IPCGETDATA 		 6
#define SYS_IPCSENDERPID	 7
#define SYS_IPCNEXTMSG		 8

/* devfs syscall */
#define SYS_DEVFSREG		 9

/* VFS syscalls */
#define SYS_VFSREG			10
#define SYS_VFSOPEN			11
#define SYS_VFSCLOSE		12
#define SYS_VFSREAD			13
#define SYS_VFSWRITE		14
#define SYS_VFSMOUNT		15
#define SYS_VFSUNMOUNT		16
#define SYS_VFSFILESIZE		17
#define SYS_VFSDIRSIZE		18
#define SYS_VFSDIRLIST		19

#endif