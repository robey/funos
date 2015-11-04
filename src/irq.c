#include <stdint.h>
#include "kernel.h"
#include "irq.h"
#include "pic.h"

static irq_handler_t irq_handlers[16] = { IRQ_HANDLER_NONE, };

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

// returns the old handler, if any.
irq_handler_t irq_set_handler(uint8_t irq, irq_handler_t handler) {
  if (irq >= 16) return IRQ_HANDLER_NONE;
  irq_handler_t old = irq_handlers[irq];
  irq_handlers[irq] = handler;
  return old;
}

// ISR handler from boot.s setup.
void isr_irq(uint32_t irq, unused uint32_t *regs) {
  irq_handler_t handler = irq_handlers[irq];
  if (handler != IRQ_HANDLER_NONE) handler(irq);
}
