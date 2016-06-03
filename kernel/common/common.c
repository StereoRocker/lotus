#include "common.h"
#include "terminal.h"

void outb(uint16_t port, uint8_t value)
{
	asm volatile ("outb %1, %0" : : "dN" (port), "a" (value));
}

void outw(uint16_t port, uint16_t value)
{
	asm volatile ("outw %1, %0" : : "dN" (port), "a" (value));
}

uint8_t inb(uint16_t port)
{
	uint8_t ret;
	asm volatile ("inb %1, %0" : "=a" (ret) : "dN" (port));
	return ret;
}

uint16_t inw(uint16_t port)
{
	uint16_t ret;
	asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
	return ret;
}

uint8_t* memcpy(uint8_t* dest, uint8_t* src, uint32_t count)
{
	for (uint32_t i = 0; i < count; i++)
		dest[i] = src[i];
	return dest;
}

uint8_t* memset(uint8_t* dest, uint8_t val, uint32_t count)
{
	for (uint32_t i = 0; i < count; i++)
		dest[i] = val;
	return dest;
}

uint16_t* memsetw(uint16_t* dest, uint16_t val, uint32_t count)
{
	for (uint32_t i = 0; i < count; i++)
		dest[i] = val;
	return dest;
}

uint32_t strlen(const char *str)
{
	uint32_t count = 0;
	while (str[count] != 0)
		count++;
	return count;
}

char* strcpy(char* dest, char* src)
{
	uint32_t count = 0;
	while (src[count] != 0)
	{
		dest[count] = src[count];
		count++;
	}
	dest[count] = 0;
	return dest;
}

char* strncpy(char* dest, char* src, uint32_t count)
{
	uint32_t i;
	
	for (i=0; i < count && src[i] != 0; i++)
		dest[i] = src[i];
	for (;i < count; i++)
		dest[i] = 0;
	
	return dest;
}

// If str1 has less characters than str2, then 0 will be returned if all the characters in str1 are in str2
int32_t strcmp(const char* str1, const char* str2)
{
	uint32_t count = 0;
	while (str1[count] == str2[count])
	{
		count++;
	}
	if (str1[count] == 0)
		return 0;
	
	if (str1[count] < str2[count])
		return -1;
	return 1;
}

void panic(const char* message, const char* file, uint32_t line, const char* function)
{
	bochs_break();
	asm volatile("cli");
	kprintf("Kernel panic: %s\n%s:%i function:%s\n", message, file, line, function);
	for (;;);
}
