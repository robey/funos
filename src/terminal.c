#include <stdint.h>
#include "kernel.h"

/*
 * try to use the serial port (COM1) to communicate.
 */

#define SERIAL1 0x3f8

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

uint16_t serial_port1() {
  return SERIAL1;
}

void serial_setup(uint16_t port) {
  // set serial port to 9600, 8N1
  uint16_t speed = DIVISOR / 9600;
  // uint8_t x;

  asm_outb(port + PORT_LINE_CONTROL, MODE_DIVISOR_LATCH | MODE_8N1);
  asm_outb(port + PORT_DIVISOR_HIGH, (uint8_t) (speed >> 8));
  asm_outb(port + PORT_DIVISOR_LOW, (uint8_t) (speed & 0xff));

  asm_outb(port + PORT_LINE_CONTROL, MODE_8N1);

  // turn on interrupt for receive
  asm_outb(port + PORT_INT_ENABLE, INT_RX | INT_TX);

  asm_outb(port + PORT_TX, 0x40);
  asm_outb(port + PORT_TX, 0x41);


  // INB(SERIAL1 + 3, x);
  // hexout(x);
  // terminal_putchar(',');
  // OUTB(SERIAL1 + 3, MODE_DIVISOR_LATCH | MODE_8N1);

}

 // // robey.
 // #define __inb(port) ({
 //   uint8_t t;
 //   asm volatile (
 //     "in %1, %0"
 //     : "=a" (t)
 //     : "d" ((uint16_t)(port))
 //   );
 //   t;
 // })
 //
 // #define __outb(port, val)
 //   asm volatile (
 //     "out %1, %0"
 //     : /* no outputs */
 //     : "d" ((uint16_t)(port)),"a" ((uint8_t)(val))
 //   )
