

SYSROOT = $(PWD)/sysroot
STAGING = $(PWD)/staging

ARCH = armv8-m
.PHONY: kernel libc utils tools staging image clean
kernel:
	$(MAKE) -C ./uCnix SYSROOT=$(SYSROOT) CONFIG=$(PWD)/kernel.config kernel
	$(MAKE) -C ./uCnix SYSROOT=$(SYSROOT) install

libc:
	$(MAKE) -C ./uCnix-libc SYSROOT=$(SYSROOT) ARCH=$(ARCH) install

utils: libc
	$(MAKE) -C ./uCnix-utils SYSROOT=$(SYSROOT) ARCH=$(ARCH) install

tools:
	$(MAKE) -C ./tools all

staging: libc utils
	cp -r ./base_root/* $(STAGING)
	cp -r $(SYSROOT)/bin/* $(STAGING)/bin

image: staging tools
	./tools/mkfs.elf -c -b 32 $(STAGING) rootfs.bin
	cat $(SYSROOT)/kernel.bin rootfs.bin > image.bin

clean:
	rm -rf $(STAGING)/*
	rm -rf $(SYSROOT)/*
	$(MAKE) -C ./uCnix SYSROOT=$(SYSROOT) clean
	$(MAKE) -C ./uCnix-libc SYSROOT=$(SYSROOT) ARCH=$(ARCH) clean
	$(MAKE) -C ./uCnix-utils SYSROOT=$(SYSROOT) ARCH=$(ARCH) clean
	rm -f rootfs.bin image.bin

