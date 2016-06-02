#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

void outb(uint16_t port, uint8_t value);
void outw(uint16_t port, uint16_t value);
uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port);
uint8_t* memcpy(uint8_t* dest, uint8_t* src, uint32_t count);
uint8_t* memset(uint8_t* dest, uint8_t val, uint32_t count);
uint16_t* memsetw(uint16_t* dest, uint16_t val, uint32_t count);
uint32_t strlen(const char *str);
char* strcpy(char* dest, char* src);
char* strncpy(char* dest, char* src, uint32_t count);
int32_t strcmp(const char* str1, const char* str2);
void panic(const char* message, const char* file, uint32_t line, const char* function);
void kprintf(const char* format, ...);

#define PANIC(msg) panic(msg, __FILE__, __LINE__, __func__)
// If this is set, then the ASSERTV macro will function
#define DEBUG

#define ASSERT(expression) if (!(expression)) PANIC("Assertion failed!");

#ifdef DEBUG
#define ASSERTV(expression) if (!(expression)) PANIC("Assertion failed!"); else kprintf("Assertion succeeded %s:%i function:%s\n", __FILE__, __LINE__, __func__);
#else
#define ASSERTV(expression) ASSERT(expression)
#endif

#define UNUSED(var) var = var

// These are defines that will work exclusively with the Bochs VM
#define bochs_putc(x) outb(0xE9, x)
#define bochs_break() asm volatile("xchgw %bx, %bx")

#endif
