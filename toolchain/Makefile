ROOT := ..

NASM_VERSION        := 2.11.08
GCC_VERSION         := 4.9.3
BINUTILS_VERSION    := 2.25

SRCDIR := $(abspath $(ROOT)/toolchain)
OBJDIR := $(abspath $(ROOT)/build/toolchain)
TOOLSDIR := $(abspath $(ROOT)/tools)
BINDIR := $(TOOLSDIR)/bin
TARGET = $(NASM_BIN) $(BINUTILS_BIN) $(GCC_BIN)

OBJDIR_IMMORTAL := true
TARGET_IMMORTAL := true

NASM_TAR := $(SRCDIR)/nasm-$(NASM_VERSION).tar.bz2
GCC_TAR := $(SRCDIR)/gcc-$(GCC_VERSION).tar.bz2
BINUTILS_TAR := $(SRCDIR)/binutils-$(BINUTILS_VERSION).tar.bz2

NASM_FOLDER := $(OBJDIR)/nasm-$(NASM_VERSION)
GCC_FOLDER := $(OBJDIR)/gcc-$(GCC_VERSION)
BINUTILS_FOLDER := $(OBJDIR)/binutils-$(BINUTILS_VERSION)

NASM_BIN = $(BINDIR)/nasm
GCC_BIN = $(BINDIR)/$(PLATFORM)-gcc
BINUTILS_BIN = $(BINDIR)/$(PLATFORM)-as

include $(ROOT)/build.mk


COMMON_CONFIG := --prefix=$(TOOLSDIR) --target=$(PLATFORM)
GCC_CONFIG := $(COMMON_CONFIG) --disable-nls --enable-languages=c,c++ --without-headers --enable-interwork --enable-multilib
BINUTILS_CONFIG := $(COMMON_CONFIG) --enable-interwork --enable-multilib --disable-nls --disable-werror --with-sysroot


$(TOOLSDIR):
	mkdir -p $(TOOLSDIR)

$(OBJDIR)/%: $(SRCDIR)/%.tar.bz2
	cd $(OBJDIR) && $(TAR) xf $<

$(NASM_BIN): $(NASM_FOLDER)
	# nasm really would like to be built in its own source folder, please.
	cp -a $(NASM_FOLDER) $(OBJDIR)/nasm
	cd $(OBJDIR)/nasm && ./configure $(COMMON_CONFIG)
	cd $(OBJDIR)/nasm && make everything && make install_everything

$(BINUTILS_BIN): $(BINUTILS_FOLDER)
	mkdir -p $(OBJDIR)/binutils
	cd $(OBJDIR)/binutils && $(BINUTILS_FOLDER)/configure $(BINUTILS_CONFIG)
	cd $(OBJDIR)/binutils && make && make install

$(GCC_BIN): $(GCC_FOLDER) $(BINUTILS_BIN)
	mkdir -p $(OBJDIR)/gcc
	cd $(OBJDIR)/gcc && $(GCC_FOLDER)/configure $(GCC_CONFIG)
	cd $(OBJDIR)/gcc && make all-gcc && make all-target-libgcc
	cd $(OBJDIR)/gcc && make install-gcc && make install-target-libgcc
