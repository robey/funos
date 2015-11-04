ROOT := .
SRCDIR := src
OBJDIR := build-kernel
TARGET := funos.bin
STARTC := startc

KERNEL := funos.bin
SOURCES_S :=            \
  boot.s                \
  idt.s                 \
  $(END)
SOURCES_C :=            \
  buffer.c              \
  cpuid.c               \
  irq.c                 \
  kernel.c              \
  pic.c                 \
  serial.c              \
  vga.c                 \
  $(END)
LIBDIRS :=              \
  boot                  \
  $(END)

# OBJECTS := $(addprefix $(TARGET)/, $(SOURCES_C:.c=.o)) $(addprefix $(TARGET)/, $(SOURCES_S:.s=.o))

include build.mk

foo: $(OBJDIR)/boot.a

$(TARGET): $(SRCDIR)/linker.ld $(OBJECTS) $(LIBS)
	$(GCC) -T $(SRCDIR)/linker.ld -o $(TARGET) $(OBJECTS) $(LIBS) $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.s
	$(GAS) $< -o $@



run: all
	qemu-system-x86_64 -s -smp cpus=2 -kernel ./funos.bin

run-serial: all
	qemu-system-i386 -s -kernel ./funos.bin -nographic

.PHONY:  run-serial run


#
# TARGET := target
# SOURCE := lcdtext.cpp
# TEST_SOURCE := test.cpp
# LIBRARY := $(TARGET)/liblcdtext.a
# BIN := test
#
# all: $(TARGET) $(LIBRARY)
#
# include build.mk
#
# $(LIBRARY): $(OBJ)
# 	$(AR) rcs $(LIBRARY) $(OBJ)
#
# test: $(TARGET) upload
#
# $(TARGET)/test.elf: $(TEST_OBJ) $(LIBRARY) $(LIBARDUINO_LIB)
# 	$(CC) $(CC_OPTIMIZE) -Wl,--gc-sections $(CC_PLATFORM) -o $@ $+
#
# .PHONY: test all
#
# -include $(TARGET)/*.d
