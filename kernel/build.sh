#!/bin/bash

# Build a list of source files
ASMFILES=$(find * -type f -name *.s)
NASMFILES=$(find * -type f -name *.ns)
CFILES=$(find * -type f -name *.c)

# Start assembling
echo Assembling sources...
for a in $ASMFILES
do
	echo Assembling $a
	i386-elf-as $a -o $(basename $a .s).o
	OBJS="$OBJS $(basename $a .s).o"
done;
echo

# Assemble NASM sources (I'm lazy)
echo Assembling NASM sources...
for n in $NASMFILES
do
	echo Assembling $n
	nasm $n -o $(basename $n .ns).o -felf32
	OBJS="$OBJS $(basename $n .ns).o"
done;
echo

# Start compiling
echo Compiling sources...
for c in $CFILES
do
	echo Compiling $c
	i386-elf-gcc -c $c -o $(basename $c .c).o -Iinclude -ffreestanding -Wall -Wextra -std=gnu99
	OBJS="$OBJS $(basename $c .c).o"
done;

# Link!
echo Linking...
i386-elf-gcc -T link.ld -o kernel.bin -ffreestanding -nostdlib $OBJS -lgcc

# Clean objects
echo Cleaning object files
rm $OBJS

# Done?
echo Done!
