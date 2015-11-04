ROOT := ../..

SRCDIR := $(ROOT)/src/boot
OBJDIR := $(ROOT)/build-kernel/boot
TARGET := $(ROOT)/build-kernel/boot.a

SOURCES_ASM :=          \
  apic.asm              \
  init.asm              \
  vga.asm               \
  $(END)

include $(ROOT)/build.mk

$(TARGET): $(OBJECTS)
	$(GAR) rvs $@ $^
