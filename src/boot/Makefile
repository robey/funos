ROOT := ../..

SRCDIR := $(abspath $(ROOT)/src/boot)
OBJDIR := $(abspath $(ROOT)/build/kernel/boot)
TARGET := $(abspath $(ROOT)/build/kernel/boot.a)

SOURCES_ASM :=          \
  crash.asm             \
  init.asm              \
	irq.asm               \
	serial.asm            \
  vga.asm               \
  $(END)

include $(ROOT)/build.mk

$(TARGET): $(OBJECTS)
	$(GAR) rvs $@ $^
