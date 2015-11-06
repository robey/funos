END :=

#PLATFORM := i686-elf
PLATFORM := x86_64-elf
BINDIR := $(abspath $(ROOT)/tools/bin)

TAR := tar
GCC := $(BINDIR)/$(PLATFORM)-gcc
GAS := $(BINDIR)/$(PLATFORM)-as
GAR := $(BINDIR)/$(PLATFORM)-ar

# FIXME
NASM := /usr/local/bin/nasm

CFLAGS := -g -O2
REAL_CFLAGS := -MMD -std=gnu99 -ffreestanding -Wall -Wextra -fvisibility=hidden $(CFLAGS)
# -isystem $(STARTC)/include
LDFLAGS := -ffreestanding -O2 -nostdlib -lgcc

OBJECTS_C := $(addprefix $(OBJDIR)/, $(SOURCES_C:.c=.o))
OBJECTS_ASM := $(addprefix $(OBJDIR)/, $(SOURCES_ASM:.asm=.o))
OBJECTS := $(OBJECTS_ASM) $(OBJECTS_C)

LIBS := $(addsuffix .a, $(addprefix $(OBJDIR)/, $(LIBDIRS)))

all: $(OBJDIR) $(TARGET)

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

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(GCC) -c $(REAL_CFLAGS) $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.asm
	$(NASM) -felf32 -o $@ $<

$(OBJDIR)/%.a: $(SRCDIR)/%/*
	$(MAKE) -C $(<D)

-include $(OBJDIR)/*.d

.PHONY: all clean
