#pragma once

#include <stdint.h>
#include "common.h"

/*
 * boot info provided by multiboot.
 */

enum {
  BFI_CMDLINE        = 1 << 2,  // command-line is present
  BFI_ELF_TABLE      = 1 << 5,  // elf table deconstructed for us
  BFI_MEMORY_MAP     = 1 << 6,  // map of ram
} boot_info_flags;

typedef struct packed {
  uint32_t size;
  uint64_t base_addr;
  uint64_t length;
  uint32_t type;
} memory_map;

// use the 'size' field on memory_map to find the next entry.
#define MMAP_NEXT(mmap) ((memory_map *) ((void *) (mmap) + (mmap)->size + 4))

// convert the base_addr into a 32-bit pointer
#define MMAP_PTR(mmap) ((char const *) ((uint32_t) ((mmap)->base_addr)))
#define MMAP_PTR_END(mmap) (MMAP_PTR(mmap) + (uint32_t) (mmap)->length)

// only one 'type' is really defined: available RAM
#define MMAP_TYPE_RAM           1

typedef struct packed {
  uint32_t flags;
  uint32_t mem_lower;
  uint32_t mem_upper;
  uint32_t boot_device;
  char *cmdline;                // BFI_CMDLINE
  uint32_t mods_count;
  void *mods_addr;

  // elf:
  uint32_t shdr_num;            // BFI_ELF_TABLE
  uint32_t shdr_size;           // BFI_ELF_TABLE
  uint32_t shdr_addr;           // BFI_ELF_TABLE
  uint32_t shdr_shndx;          // BFI_ELF_TABLE

  uint32_t mmap_length;         // BFI_MEMORY_MAP
  memory_map *mmap_addr;        // BFI_MEMORY_MAP
} boot_info;

// use 'mmap_addr' and 'mmap_length' to determine if we're still in the memory map area.
#define MMAP_VALID(bi, mmap) ((void *) (mmap) < ((void *) ((bi)->mmap_addr) + (bi)->mmap_length))
