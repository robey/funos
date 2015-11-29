#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
//#include "loader.h"
#include "log.h"
#include "mp.h"

static bool mpd_checksum_valid(mp_descriptor const *mpd) {
  const char *p = (const char *) mpd;
  uint8_t checksum = 0;
  for (int i = 0; i < 16; i++) checksum += p[i];
  return (checksum == 0);
}

// find the "_MP_" descriptor by scanning reserved memory.
// that is legitimately what you are supposed to do: just dig through the
// trash until you find something shiny.
static mp_descriptor const *find_mp_block(boot_info const *bi) {
  log("Searching for processor map... \\");
  for (memory_map const *mmap = bi->mmap_addr; MMAP_VALID(bi, mmap); mmap = MMAP_NEXT(mmap)) {
    if (mmap->type == MMAP_TYPE_RAM) continue;
    // it *must* be in the lower 32 bits of address space.
    if ((mmap->base_addr >> 32) != 0) continue;

    for (char const *p = MMAP_PTR(mmap); p < MMAP_PTR_END(mmap); p += 16) {
      mp_descriptor const *mpd = (mp_descriptor const *) p;
      if (mpd->signature != MPD_SIGNATURE) continue;
      if (!mpd_checksum_valid(mpd)) continue;
      if (mpd->length != MPD_LENGTH || mpd->spec_rev != MPD_SPEC_1_4) continue;
      log("found valid MP block at {xd}", p);
      return mpd;
    }
  }
  return NULL;
}

static bool mpc_checksum_valid(mp_config const *mpc) {
  const char *p = (const char *) mpc;
  uint8_t checksum = 0;
  for (int i = 0; i < mpc->table_length; i++) checksum += p[i];
  return (checksum == 0);
}

mp_config const *find_mp_config(boot_info const *bi) {
  mp_descriptor const *mpd = find_mp_block(bi);
  if (mpd == NULL) return NULL;
  mp_config const *mpc = mpd->config;
  if (!mpc_checksum_valid(mpc)) return NULL;

  log("Your OEM: {s:8:8} {s:12:12}", mpc->oem_id, mpc->product_id);
  log("Local APIC at {xd}", mpc->local_apic_addr);

  // scan the "entries" after this.
  char const *p = (char const *) mpc + sizeof(mp_config);
  for (int i = 0; i < mpc->entry_count; i++) {
    switch (*p) {
      case MPC_TYPE_CPU: {
        mp_config_cpu const *cpu = (mp_config_cpu const *) p;
        bool enabled = (cpu->flags & MPC_CPU_FLAG_ENABLED) != 0;
        bool boot = (cpu->flags & MPC_CPU_FLAG_BOOT) != 0;
        log("  CPU {d} {s} {s}", cpu->apic_id, enabled ? "ok" : "DISABLED", boot ? "boot" : "");
        p += sizeof(mp_config_cpu);
        break;
      }
      case MPC_TYPE_BUS: {
        mp_config_bus const *bus = (mp_config_bus const *) p;
        log("  bus {d} {s:6:6}", bus->bus_id, bus->name);
        p += sizeof(mp_config_bus);
        break;
      }
      case MPC_TYPE_IO_APIC: {
        mp_config_io_apic const *apic = (mp_config_io_apic const *) p;
        bool enabled = (apic->flags & MCP_IO_APIC_FLAG_ENABLED) != 0;
        log("  I/O APIC {d} {s} at {xd}", apic->apic_id, enabled ? "ok" : "DISABLED", apic->addr);
        p += sizeof(mp_config_io_apic);
        break;
      }
      case MPC_TYPE_IO_INT: {
        mp_config_int const *intr = (mp_config_int const *) p;
        log("  I/O int type {d} from bus {d} irq {d} to APIC {d} pin {d}",
          intr->int_type, intr->source_bus_id, intr->source_bus_irq, intr->dest_apic_id, intr->dest_apic_pin);
        p += sizeof(mp_config_int);
        break;
      }
      case MPC_TYPE_LOCAL_INT: {
        mp_config_int const *intr = (mp_config_int const *) p;
        log("  local int type {d} from bus {d} irq {d} to APIC {d} pin {d}",
          intr->int_type, intr->source_bus_id, intr->source_bus_irq, intr->dest_apic_id, intr->dest_apic_pin);
        p += sizeof(mp_config_int);
        break;
      }
    }
  }

  return mpc;
}
