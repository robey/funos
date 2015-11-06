ROOT := .

OBJDIR := build
OBJDIR_IMMORTAL := true

include $(ROOT)/build.mk


all: $(ROOT)/funos.bin

clean::
	$(MAKE) -C src clean

distclean: clean
	rm -rf tools

$(ROOT)/funos.bin: $(ROOT)/tools/bin/$(PLATFORM)-gcc
	$(MAKE) -C src

$(ROOT)/tools/bin/%:
	$(MAKE) -C toolchain

run: all
	qemu-system-x86_64 -s -smp cpus=2 -kernel $(ROOT)/funos.bin

run-serial: all
	qemu-system-i386 -s -kernel $(ROOT)/funos.bin -nographic

.PHONY: run-serial run

