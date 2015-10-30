#include <stdint.h>
#include "kernel.h"
#include "irq.h"
#include "pic.h"

void irq_enable(uint32_t irq) {
  uint8_t data;
  uint16_t port = PIC1_DATA;

  if (irq >= 8) {
    port = PIC2_DATA;
    irq -= 8;
  }

  asm_cli();
  asm_inb(port, data);
  asm_outb(port, data & ~(1 << irq));
  asm_sti();
}

void irq_disable(uint32_t irq) {
  uint8_t data;
  uint16_t port = PIC1_DATA;

  if (irq >= 8) {
    port = PIC2_DATA;
    irq -= 8;
  }

  asm_cli();
  asm_inb(port, data);
  asm_outb(port, data | (1 << irq));
  asm_sti();
}
