END :=

PLATFORM := i686-elf
NASM_PLATFORM := elf32
# PLATFORM := x86_64-elf
# NASM_PLATFORM := elf64
BINDIR := $(abspath $(ROOT)/tools/bin)

TAR := tar
NASM := $(BINDIR)/nasm
GCC := $(BINDIR)/$(PLATFORM)-gcc
GAS := $(BINDIR)/$(PLATFORM)-as
GAR := $(BINDIR)/$(PLATFORM)-ar
OBJDUMP := $(BINDIR)/$(PLATFORM)-objdump

# so we can depend on them, and use these dependencies to build them if necessary:
BASIC_TOOLS := $(NASM) $(GCC) $(GAR)

CFLAGS := -g -O2
MY_CFLAGS := -MMD -std=gnu99 -ffreestanding -Wall -Wextra -fvisibility=hidden
EXTRA_X64_CFLAGS := -mcmodel=small -mno-red-zone -mno-mmx -mno-sse -mno-sse2
TOTAL_CFLAGS_32 := $(MY_CFLAGS) $(CFLAGS)
TOTAL_CFLAGS_64 := $(MY_CFLAGS) $(EXTRA_X64_CFLAGS) $(CFLAGS)
TOTAL_CFLAGS := TOTAL_CFLAGS_64

# -isystem $(STARTC)/include
LDFLAGS := -ffreestanding -O2 -nostdlib -lgcc

OBJECTS_C := $(addprefix $(OBJDIR)/, $(SOURCES_C:.c=.o))
OBJECTS_ASM := $(addprefix $(OBJDIR)/, $(SOURCES_ASM:.asm=.o))
OBJECTS := $(OBJECTS_ASM) $(OBJECTS_C)

LIBS := $(abspath $(addsuffix .a, $(addprefix $(OBJDIR)/, $(LIBDIRS))))

all: $(BASIC_TOOLS) $(OBJDIR) $(TARGET)

$(LIBS): $(foreach lib, $(LIBDIRS), $(lib)/*)

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean::
ifndef TARGET_IMMORTAL
ifdef TARGET
	rm -rf $(TARGET)
endif
endif
ifndef OBJDIR_IMMORTAL
ifdef OBJDIR
	rm -rf $(OBJDIR)
endif
endif
	$(foreach dir,$(LIBDIRS),$(MAKE) -C $(dir) clean)

distclean:: clean
	rm -rf $(TARGET) $(OBJDIR)
	$(foreach dir,$(LIBDIRS),$(MAKE) -C $(dir) distclean)

$(BASIC_TOOLS):
	$(MAKE) -C toolchain

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(GCC) -c $(TOTAL_CFLAGS) $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.asm
	$(NASM) -f$(NASM_PLATFORM) -o $@ $<

$(OBJDIR)/%.o: $(SRCDIR)/%.asm32
	$(NASM) -felf32 -o $@ $<

$(OBJDIR)/%.a: $(SRCDIR)/%/*
	$(MAKE) -C $(<D)

-include $(OBJDIR)/*.d

.PHONY: all clean
