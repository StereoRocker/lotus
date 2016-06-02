CC:=i386-elf-gcc
AS:=i386-elf-as
NASM:=nasm

PWD:=$(shell pwd)
INCLUDE:=$(PWD)/include
CFLAGS:=-I$(INCLUDE) -ffreestanding -Wall -Wextra -std=gnu99
ASFLAGS:=
LDFLAGS:=-T link.ld -ffreestanding -nostdlib -lgcc
NASMFLAGS:=-felf32

ASMFILES:=$(shell find * -type f -name *.s -printf "%p ")
NASMFILES:=$(shell find * -type f -name *.ns -printf "%p ")
CFILES:=$(shell find * -type f -name *.c -printf "%p ")
SOURCES:=$(ASMFILES) $(NASMFILES) $(CFILES)

ASMOBJECTS:=$(ASMFILES:.s=.o)
NASMOBJECTS:=$(NASMFILES:.ns=.o) 
COBJECTS:=$(CFILES:.c=.o)
OBJECTS:=$(ASMOBJECTS) $(NASMOBJECTS) $(COBJECTS)

EXECUTABLE:=kernel.bin

all: $(ASMFILES) $(NASMFILES) $(CFILES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

%.o: %.ns
	$(NASM) $(NASMFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS)

.PHONY: clean