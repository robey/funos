#include <stdbool.h>
#include <stdint.h>
#include "loader.h"
#include "log.h"
#include "rtc.h"
#include "vgaterm.h"

export void loader(void) {
  vgaterm_init();
  log("FunOS (c) 2015-2109 Regents of Teeth-gnashing Despair");
  log("32-bit bootloader: detecting VM sandbox");
  log("Sleeping for 500 milliseconds to see if I can... \\");
  timer_sleep(500);
  log("I awoke!");

  rtc_clock clock;
  log("Fetching current date/time: \\");
  rtc_get(&clock);
  log("{w}-{b0:2}-{b0:2} {b0:2}:{b0:2}:{b0:2}",
    clock.year, clock.month, clock.day, clock.hour, clock.minute, clock.second);
  vgaterm_set_clock(&clock);

  uint16_t count = 0;
  while (true) {
    count++;
    timer_sleep(1000);
    log("What's up, doc? {w}", count);
  }
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
