#ifndef ELF_H
#define ELF_H

typedef struct {
	uint8_t magic[4];
	uint8_t bits;
	uint8_t endianness;
	uint8_t elf_version;
	uint8_t ABI;
	uint8_t padding[8];
	uint16_t type;
	uint16_t machine;
	uint32_t elf_version_long;
	uint32_t prog_entry;
	uint32_t prog_header;
	uint32_t sec_header;
	uint32_t flags;
	uint16_t header_size;
	uint16_t phead_ent_size;
	uint16_t phead_ents;
	uint16_t shead_ent_size;
	uint16_t shead_ents;
	uint16_t shead_name_index;
} elf_header_t;

typedef struct {
	uint32_t segtype;
	uint32_t p_offset;
	uint32_t vaddr;
	uint32_t undef;
	uint32_t p_filesz;
	uint32_t p_memsz;
	uint32_t flags;
	uint32_t align;
} elf_pheader_t;

#define ELF_EXEC 0x1
#define ELF_WRITE 0x2
#define ELF_READ 0x4

#define ELF_X86 0x3

#endif