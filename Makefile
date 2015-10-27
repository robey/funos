END :=

SOURCE := src
TARGET := build-kernel
STARTC := startc

KERNEL := funos.bin
SOURCES_S :=            \
  boot.s                \
	idt.s                 \
	pic.s                 \
	$(END)
SOURCES_C :=            \
  buffer.c              \
  cpuid.c               \
  terminal.c            \
  kernel.c              \
  vga.c                 \
  $(END)

OBJECTS := $(addprefix $(TARGET)/, $(SOURCES_C:.c=.o)) $(addprefix $(TARGET)/, $(SOURCES_S:.s=.o))

GCC := target/bin/i686-elf-gcc
GAS := target/bin/i686-elf-as

CFLAGS := -std=gnu99 -ffreestanding -O2 -Wall -Wextra -fvisibility=hidden -isystem $(STARTC)/include
LDFLAGS := -ffreestanding -O2 -nostdlib -lgcc

all: $(KERNEL)

clean:
	rm -rf $(TARGET) $(KERNEL)

$(KERNEL): $(SOURCE)/linker.ld $(TARGET) $(OBJECTS)
	$(GCC) -T $(SOURCE)/linker.ld -o $(KERNEL) $(OBJECTS) $(LDFLAGS)

$(TARGET):
	mkdir -p $(TARGET)

$(TARGET)/%.s: $(SOURCE)/%.c
	$(GCC) -S $(CFLAGS) $< -o $@

$(TARGET)/%.o: $(TARGET)/%.s
	$(GAS) $< -o $@

$(TARGET)/%.o: $(SOURCE)/%.s
	$(GAS) $< -o $@

.PHONY: all clean


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
