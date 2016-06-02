#ifndef EXEC_H
#define EXEC_H

// In switch.ns
extern void kexec_jmp(uint32_t stack, uint32_t entry);

void kexec_elf(uint8_t* mem, uint32_t length);

#endif