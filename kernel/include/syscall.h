#ifndef SYSCALL_H
#define SYSCALL_H

typedef uint32_t (*syscall)(uint32_t data);
void syscall_init();
uint32_t syscall_call(uint32_t call, uint32_t data);
void syscall_register(syscall func, uint32_t call);

#endif