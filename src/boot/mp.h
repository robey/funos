#pragma once

#include <stdint.h>
#include "boot_info.h"
#include "common.h"

typedef struct packed {
  uint32_t signature;
  uint16_t table_length;
  uint8_t spec_rev;
  uint8_t checksum;
  char oem_id[8];
  char product_id[12];
  void *oem_table;
  uint16_t oem_table_size;
  uint16_t entry_count;
  void *local_apic_addr;
  uint16_t extended_table_length;
  uint8_t extended_table_checksum;
  uint8_t reserved;
} mp_config;

#define MPC_SIGNATURE           0x504d4350

#define MPC_TYPE_CPU            0
#define MPC_TYPE_BUS            1
#define MPC_TYPE_IO_APIC        2
#define MPC_TYPE_IO_INT         3
#define MPC_TYPE_LOCAL_INT      4

typedef struct packed {
  uint8_t type;
  uint8_t apic_id;
  uint8_t apic_version;
  uint8_t flags;
  uint32_t signature;
  uint32_t feature_flags;
  uint32_t reserved[2];
} mp_config_cpu;

#define MPC_CPU_FLAG_ENABLED    (1 << 0)
#define MPC_CPU_FLAG_BOOT       (1 << 1)

typedef struct packed {
  uint8_t type;
  uint8_t bus_id;
  char name[6];
} mp_config_bus;

typedef struct packed {
  uint8_t type;
  uint8_t apic_id;
  uint8_t apic_version;
  uint8_t flags;
  void *addr;
} mp_config_io_apic;

#define MCP_IO_APIC_FLAG_ENABLED    (1 << 0)

typedef struct packed {
  uint8_t type;
  uint8_t int_type;
  uint16_t flags;
  uint8_t source_bus_id;
  uint8_t source_bus_irq;
  uint8_t dest_apic_id;
  uint8_t dest_apic_pin;
} mp_config_int;

// -----

typedef struct paragraph_aligned packed {
  uint32_t signature;
  mp_config *config;
  uint8_t length;
  uint8_t spec_rev;
  uint8_t checksum;
  uint8_t features[5];
} mp_descriptor;

#define MPD_SIGNATURE           0x5f504d5f
#define MPD_SPEC_1_0            1
#define MPD_SPEC_1_4            4
#define MPD_LENGTH              1

mp_config const *find_mp_config(boot_info const *bi);
