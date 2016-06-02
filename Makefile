all: newlib kernel testprog iso

rebuild: clean all

newlib:
	./regen_newlib.sh
	./copy_headers.sh

kernel:
	$(MAKE) -C kernel

testprog: newlib
	$(MAKE) -C testprog

iso: kernel testprog
	mkdir isodir
	mkdir isodir/boot
	mkdir isodir/boot/grub
	
	cp sysroot/boot/grub/grub.cfg isodir/boot/grub/grub.cfg
	cp kernel/kernel.bin isodir/boot/kernel.bin
	cp testprog/test.elf isodir/boot/test.elf
	
	grub-mkrescue -o os.iso -d /usr/lib/grub/i386-pc isodir
	
	rm -rf isodir
	
clean:
	$(MAKE) -C kernel clean
	$(MAKE) -C testprog clean

.PHONY: all clean kernel testprog