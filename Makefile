ROOT := .

OBJDIR := build
OBJDIR_IMMORTAL := true

include $(ROOT)/build.mk

QEMU_OPTIONS := -smp cpus=2 -rtc base=localtime,clock=host


all: $(ROOT)/funos.bin

clean::
	$(MAKE) -C src clean

distclean::
	rm -rf tools

$(ROOT)/funos.bin: src/* src/*/*
	$(MAKE) -C src

run: all
	qemu-system-x86_64 -s $(QEMU_OPTIONS) -kernel $(ROOT)/funos.bin

run-serial: all
	qemu-system-x86_64 -s $(QEMU_OPTIONS) -kernel $(ROOT)/funos.bin -nographic

dump: all
	$(OBJDUMP) -t funos.bin -d -M intel

.PHONY: run-serial run
