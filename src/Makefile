ROOT := ..

SRCDIR := $(abspath $(ROOT)/src)
OBJDIR := $(abspath $(ROOT)/build/kernel)
TARGET := $(abspath $(ROOT)/funos.bin)

# SOURCES_S :=            \
#   boot.s                \
#   # idt.s                 \
#   $(END)
# SOURCES_C :=            \
#   buffer.c              \
#   cpuid.c               \
#   irq.c                 \
#   kernel.c              \
#   pic.c                 \
#   serial.c              \
#   vga.c                 \
#   $(END)
LIBDIRS :=              \
  boot                  \
  $(END)

include $(ROOT)/build.mk

$(TARGET): $(SRCDIR)/linker.ld $(OBJECTS) $(LIBS)
	$(GCC) -T $(SRCDIR)/linker.ld -o $(TARGET) $(OBJECTS) $(LIBS) $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.s
	$(GAS) $< -o $@
