ROOT := ../..

SRCDIR := $(abspath $(ROOT)/src/boot)
OBJDIR := $(abspath $(ROOT)/build/kernel/boot)
TARGET := $(abspath $(ROOT)/build/kernel/boot.a)

SOURCES_ASM :=          \
  crash.asm             \
  keyboard.asm          \
  init.asm              \
  irq.asm               \
  serial.asm            \
  timer.asm             \
  vga.asm               \
  $(END)

SOURCES_C :=            \
  loader.c              \
  log.c                 \
  mp.c                  \
  rtc.c                 \
  vgaterm.c             \
  $(END)

include $(ROOT)/build.mk

# boot-loader is in 32-bit mode.
override TOTAL_CFLAGS := $(TOTAL_CFLAGS_32)

$(TARGET): $(OBJECTS)
	$(GAR) rvs $@ $^
