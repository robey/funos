#include <stdbool.h>
#include <stdint.h>
#include "boot_info.h"
#include "common.h"
#include "loader.h"
#include "log.h"
#include "mp.h"
#include "rtc.h"
#include "vgaterm.h"

export void loader(void) {
  vgaterm_init();
  log("FunOS (c) 2015-2109 Regents of Teeth-gnashing Despair");
  log("32-bit bootloader: detecting VM sandbox");
  log("Sleeping for 50msec to test timer... \\");
  timer_sleep(50);
  log("I woke up!");

  rtc_clock clock;
  log("Fetching current date/time: \\");
  rtc_get(&clock);
  log("{w}-{b0:2}-{b0:2} {b0:2}:{b0:2}:{b0:2}",
    clock.year, clock.month, clock.day, clock.hour, clock.minute, clock.second);
  vgaterm_set_clock(&clock);

  boot_info *bi = get_boot_info();
  if (!(bi->flags & BFI_MEMORY_MAP)) {
    log("No memory map?!");
    return;
  }

  log("Memory map:");
  for (const memory_map *mmap = bi->mmap_addr; MMAP_VALID(bi, mmap); mmap = MMAP_NEXT(mmap)) {
    uint64_t start = mmap->base_addr;
    uint64_t end = start + mmap->length;
    if (mmap->type == MMAP_TYPE_RAM) {
      uint64_t kb = mmap->length / 1024;
      if (kb > 2048) {
        uint64_t mb = kb / 1024;
        log("  {xq}-{xq} {q}MB", start, end, mb);
      } else {
        log("  {xq}-{xq} {q}KB", start, end, kb);
      }
    } else {
      log("  {xq}-{xq} reserved", start, end);
    }
  }

  find_mp_config(bi);
  // uint16_t count = 0;
  // while (true) {
  //   count++;
  //   timer_sleep(1000);
  //   log("What's up, doc? {w}", count);
  // }
}

export void event_keyboard(uint32_t scancode) {
  vgaterm_keyboard(scancode);
}

export void event_serial(uint32_t data) {
  vgaterm_write(data);
  serial_write(data);
}

export void event_second_timer(void) {
  vgaterm_add_second();
}
