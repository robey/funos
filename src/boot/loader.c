#include <stdbool.h>
#include <stdint.h>
#include "loader.h"
#include "log.h"
#include "vgaterm.h"

export void loader(void) {
  vgaterm_init();
  log("Welcome to FunOS!");
  uint64_t q = 0x12345678abcd90efLL;
  log("Detecting {<s:20} {bx} {dx} {qx} ...", "troglodytes", 29, &loader, q);

  while (true);
}

export void event_keyboard(uint32_t scancode) {
  vgaterm_keyboard(scancode);
}

export void event_serial(uint32_t data) {
  vgaterm_write(data);
  serial_write(data);
}
