/* NOTE: These headers are all provided by newlib */
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/times.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdint.h>
 
void _exit();
int close(int file);
char **environ; /* Pointer to array of char * strings that define the current environment variables */
int execve(char *name, char **argv, char **env);
int fork();
int fstat(int file, struct stat *st);
int getpid();
int isatty(int file);
int kill(int pid, int sig);
int link(char *old, char *new);
int lseek(int file, int ptr, int dir);
int open(const char *name, int flags, ...);
int read(int file, char *ptr, int len);
caddr_t sbrk(int incr);
int stat(const char *file, struct stat *st);
clock_t times(struct tms *buf);
int unlink(char *name);
int wait(int *status);
int write(int file, char *ptr, int len);
//int gettimeofday(struct timeval *p, struct timezone *z);

uint32_t perf_syscall(uint32_t call, uint32_t data)
{
	uint32_t ret;
	asm volatile("mov %1, %%eax; mov %2, %%edx; int $0x80; mov %%eax, %0;" : "=r"(ret) : "r"(call), "r"(data));
	return ret;
}

void _exit()
{
	// We call the exit syscall (we hardcode it as 2 until we integrate Lotus' headers properly)
	perf_syscall(2, 0);
}

int close(int file)
{
	return -1;
}

int execve(char *name, char **argv, char **env)
{
	return -1;
}

int fork()
{
	return -1;
}

int fstat(int file, struct stat *st)
{
	return -1;
}

int getpid()
{
	return -1;
}

int isatty(int file)
{
	return -1;
}

int kill(int pid, int sig)
{
	return -1;
}

int link(char *old, char *new)
{
	return -1;
}

int lseek(int file, int ptr, int dir)
{
	return -1;
}

int open(const char *name, int flags, ...)
{
	return -1;
}

int read(int file, char *ptr, int len)
{
	return -1;
}

caddr_t sbrk(int incr)
{
	caddr_t ret = (caddr_t)perf_syscall(0, incr);
	return ret;
}

int stat(const char *file, struct stat *st)
{
	return -1;
}

clock_t times(struct tms *buf)
{
	clock_t clock;
	return clock;
}

int unlink(char *name)
{
	return -1;
}

int wait(int *status)
{
	return -1;
}

int write(int file, char *ptr, int len)
{
	// This is a temporary implementation to allow printf() from user code
	if (file == 1 || file == 2)
	{
		// We're trusting user code... this will definitely change.
		perf_syscall(0xDEADBEEF, (uint32_t)ptr);
		return len;
	}
	
	errno = EACCES;
	return -1;
}

/*int gettimeofday(struct timeval *p, struct timezone *z)
{
	return -1;
}*/