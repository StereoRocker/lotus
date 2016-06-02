#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include <stdint.h>

#define MULTIBOOT_MEM		0x001
#define MULTIBOOT_DEVICE	0x002
#define MULTIBOOT_CMDLINE	0x004
#define MULTIBOOT_MODS		0x008
#define MULTIBOOT_AOUT		0x010
#define MULTIBOOT_ELF		0x020
#define MULTIBOOT_MMAP		0x040
#define MULTIBOOT_CONFIG	0x080
#define MULTIBOOT_LOADER	0x100
#define MULTIBOOT_APM		0x200
#define MULTIBOOT_VBE		0x400

typedef struct {
	uint32_t flags;
	uint32_t mem_lower;
	uint32_t mem_upper;
	uint32_t boot_device;
	uint32_t cmdline;
	uint32_t mods_count;
	uint32_t mods_addr;
	uint32_t elf1;
	uint32_t elf2;
	uint32_t elf3;
	uint32_t elf4;
	uint32_t mmap_length;
	uint32_t mmap_addr;
	uint32_t drives_length;
	uint32_t drives_addr;
	uint32_t config;
	uint32_t bootloader_name;
	uint32_t apm_table;
	uint32_t vbe_control_info;
	uint32_t vbe_mode_info;
	uint32_t vbe_mode;
	uint32_t vbe_int_seg;
	uint32_t vbe_int_off;
	uint32_t vbe_int_len;
} multiboot_t;

typedef struct {
	uint32_t size;
	uint32_t base;
	uint32_t base_ext;
	uint32_t length;
	uint32_t length_ext;
	uint32_t type;
} mmap_entry_t;

#endif
