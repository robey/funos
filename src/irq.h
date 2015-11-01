#pragma once

#include <stdint.h>

// silly list of old IRQs.
enum {
  IRQ_TIMER = 0,
  IRQ_KEYBOARD = 1,
  IRQ_SERIAL2 = 3,
  IRQ_SERIAL1 = 4,
  IRQ_PARALLEL = 7,
  IRQ_CLOCK = 8
} irqs;

typedef void (*irq_handler_t)(uint8_t irq);
#define IRQ_HANDLER_NONE ((irq_handler_t)0)

void irq_enable(uint32_t irq);
void irq_disable(uint32_t irq);
irq_handler_t irq_set_handler(uint8_t irq, irq_handler_t handler);
void _isr_irq(uint32_t irq, unused uint32_t *regs);
