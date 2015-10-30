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

void irq_enable(uint32_t irq);
void irq_disable(uint32_t irq);
