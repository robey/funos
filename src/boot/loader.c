#include <stdbool.h>
#include <stdint.h>
#include "loader.h"
#include "vgaterm.h"

#define LF 10
#define CR 13

static void log(const char *text) {
  for (char *p = text; *p; p++) {
    vgaterm_write(*p);
    serial_write(*p);
  }
  vgaterm_write(LF);
  serial_write(CR);
  serial_write(LF);
}

export void loader(void) {
  vgaterm_init();
  log("Welcome to FunOS!");
  log("Detecting troglodytes...");

  while (true);
}

export void event_keyboard(uint32_t scancode) {
  vgaterm_keyboard(scancode);
}

export void event_serial(uint32_t data) {
  vgaterm_write(data);
  serial_write(data);
}
