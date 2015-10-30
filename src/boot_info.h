#pragma once

#include <stdint.h>

/*
 * boot info provided by multiboot.
 */

enum {
  BFI_CMDLINE        = 1 << 2,  // command-line is present
  BFI_ELF_TABLE      = 1 << 5,  // elf table deconstructed for us
  BFI_MEMORY_MAP     = 1 << 6,  // map of ram
} boot_info_flags;

typedef struct {
  uint32_t size;
  uint64_t base_addr;
  uint64_t length;
  uint32_t type;
} memory_map;

typedef struct {
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

extern boot_info *_get_boot_info();
