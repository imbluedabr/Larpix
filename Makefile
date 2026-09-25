

SYSROOT = ./sysroot
ARCH = armv8-m
.PHONY: kernel libc utils tools clean
kernel:
	$(MAKE) -C ./uCnix-kernel SYSROOT=$(SYSROOT) kernel
	$(MAKE) -C ./uCnix-kernel SYSROOT=$(SYSROOT) install

libc:
	$(MAKE) -C ./uCnix-libc SYSROOT=$(SYSROOT) ARCH=$(ARCH) install

utils:
	$(MAKE) -C ./uCnix-utils SYSROOT=$(SYSROOT) ARCH=$(ARCH) install

tools:
	$(MAKE) -C ./tools all

clean:
	$(MAKE) -C ./uCnix-kernel SYSROOT=$(SYSROOT) clean
	$(MAKE) -C ./uCnix-libc SYSROOT=$(SYSROOT) ARCH=$(ARCH) clean
	$(MAKE) -C ./uCnix-utils SYSROOT=$(SYSROOT) ARCH=$(ARCH) install

