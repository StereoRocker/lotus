#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

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

int main(int argc, char**argv)
{
	// Check an ELF file was passed to the program
	if (argc < 2)
	{
		printf("You must pass the location of the ELF file to parse\n");
		return 1;
	}
	
	// Read in the ELF header
	elf_header_t elf;
	
	FILE* f = fopen(argv[1], "r");
	
	fread(&elf, sizeof(elf_header_t), 1, f);
	
	fclose(f);
	
	// Output stats from ELF file (hexadecimal)
	printf("Magic: %x %x %x %x\n", elf.magic[0], elf.magic[1], elf.magic[2], elf.magic[3]);
	printf("Type: %x\n", elf.type);
	printf("Machine: %x\n", elf.machine);
	printf("Entry: %x\n", elf.prog_entry);
	printf("Prog header position: %x\n", elf.prog_header);
	printf("Prog header entry size (dec): %i\n", elf.phead_ent_size);
	
	// Load program headers and output information about them progressively
	f = fopen(argv[1], "r");
	fseek(f, elf.prog_header, SEEK_SET);
	
	int i = 0;
	elf_pheader_t* phead = (elf_pheader_t*)malloc(elf.phead_ent_size);
	for (i = 0; i < elf.phead_ents; i++)
	{
		fread (phead, elf.phead_ent_size, 1, f);
		printf("Program header %i\n", i);
		printf("segtype: %x\n", phead->segtype);
		printf("p_offset: %x\n", phead->p_offset);
		printf("vaddr: %x\n", phead->vaddr);
		printf("p_filesz: %x\n", phead->p_filesz);
		printf("p_memsz: %x\n", phead->p_memsz);
		printf("flags: %x\n", phead->flags);
		printf("align (dec): %i\n\n", phead->align);
	}
	free(phead);
	fclose(f);
}