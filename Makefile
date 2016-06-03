all: newlib kernel testfs testdev iso

rebuild: clean all

newlib:
	./regen_newlib.sh
	./copy_headers.sh

kernel:
	$(MAKE) -C kernel

testfs: newlib
	$(MAKE) -C testfs

testdev: newlib
	$(MAKE) -C testdev

iso: kernel testfs testdev
	mkdir isodir
	mkdir isodir/boot
	mkdir isodir/boot/grub
	
	cp sysroot/boot/grub/grub.cfg isodir/boot/grub/grub.cfg
	cp kernel/kernel.bin isodir/boot/kernel.bin
	cp testfs/testfs.elf isodir/boot/testfs.elf
	cp testdev/testdev.elf isodir/boot/testdev.elf
	
	grub-mkrescue -o os.iso -d /usr/lib/grub/i386-pc isodir
	
	rm -rf isodir
	
clean:
	$(MAKE) -C kernel clean
	$(MAKE) -C testfs clean
	$(MAKE) -C testdev clean

.PHONY: all clean kernel testfs testdev
