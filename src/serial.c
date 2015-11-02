#include <stdint.h>
#include "kernel.h"
#include "irq.h"
#include "serial.h"

/*
 * try to use the serial port (COM1) to communicate.
 */

#define SERIAL1_PORT 0x3f8
#define SERIAL1_IRQ  4

#define MODE_8N1 0x03
#define MODE_DIVISOR_LATCH 0x80

#define DIVISOR 115200

#define PORT_TX             0  // w
#define PORT_RX             0  // r
#define PORT_DIVISOR_LOW    0  // rw latch
#define PORT_INT_ENABLE     1  // rw
#define PORT_DIVISOR_HIGH   1  // rw latch
#define PORT_INT_ID         2  // r
#define PORT_FIFO_CONTROL   2  // w
#define PORT_LINE_CONTROL   3  // rw
#define PORT_MODEM_CONTROL  4  // rw
#define PORT_LINE_STATUS    5  // r
#define PORT_MODEM_STATUS   6  // r
#define PORT_SCRATCH        7  // rw

#define INT_TX (1 << 1)
#define INT_RX (1 << 0)

#define STATUS_INT (1 << 0)
#define STATUS_REASON (7 << 1)
#define STATUS_REASON_TX  (1 << 1)
#define STATUS_REASON_RX  (2 << 1)

#define LINE_STATUS_TX_READY (1 << 5)

static irq_handler_t previous_irq_handler = IRQ_HANDLER_NONE;
static serial_handler_t serial_handler = SERIAL_HANDLER_NONE;
static uint16_t serial_port = 0;

uint16_t serial1_port() {
  return SERIAL1_PORT;
}

uint8_t serial1_irq() {
  return SERIAL1_IRQ;
}

static void irq_handler(unused uint8_t irq) {
  uint8_t status;

  asm_inb(serial_port + PORT_INT_ID, status);
  if ((status & STATUS_INT) == 0) {
    if ((status & STATUS_REASON) == STATUS_REASON_RX) {
      uint8_t data;
      asm_inb(serial_port + PORT_RX, data);
      if (serial_handler != SERIAL_HANDLER_NONE) serial_handler(data);
    }
  }

  if (previous_irq_handler != IRQ_HANDLER_NONE) previous_irq_handler(irq);
}

void serial_setup(uint16_t port, uint8_t irq, serial_handler_t handler) {
  // set serial port to 9600, 8N1
  uint16_t speed = DIVISOR / 9600;

  asm_outb(port + PORT_LINE_CONTROL, MODE_DIVISOR_LATCH | MODE_8N1);
  asm_outb(port + PORT_DIVISOR_HIGH, (uint8_t) (speed >> 8));
  asm_outb(port + PORT_DIVISOR_LOW, (uint8_t) (speed & 0xff));

  asm_outb(port + PORT_LINE_CONTROL, MODE_8N1);

  // turn on interrupt for receive
  asm_outb(port + PORT_INT_ENABLE, INT_RX);

  previous_irq_handler = irq_set_handler(irq, irq_handler);
  irq_enable(irq);

  serial_port = port;
  serial_handler = handler;
}

void serial_write(uint8_t data) {
  uint8_t status;
  do {
    asm_inb(serial_port + PORT_LINE_STATUS, status);
  } while ((status & LINE_STATUS_TX_READY) == 0);
  asm_outb(serial_port + PORT_TX, data);
}
