#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "cpuid.h"
#include "kernel.h"
#include "vga.h"

void print_hex8(uint8_t n) {
#define __hex_digit(n) ((n) < 10 ? '0' + (n) : 'a' + (n) - 10)
  vga_put(__hex_digit((n >> 4) & 0xf));
  vga_put(__hex_digit(n & 0xf));
}

void print_hex16(uint16_t n) {
  print_hex8(n >> 8);
  print_hex8(n);
}

void print_hex32(uint32_t n) {
  print_hex16(n >> 16);
  print_hex16(n);
}

export void _isr_irq(uint32_t irq, unused uint32_t regs) {
  vga_put('$');
  print_hex8(irq);
}

// oh no.
#define PIC1		0x20		/* IO base address for master PIC */
#define PIC2		0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)
void enable_irq(uint8_t irq) {
  uint16_t port = PIC1_DATA;

  if (irq >= 8) {
    port = PIC2_DATA;
    irq -= 8;
  }
  uint8_t x;
  asm_inb(port, x);
  x &= ~(1 << irq);
  asm_outb(port, x);
}

#include "terminal.h"
export void kernel_main() {
	vga_clear();
	vga_puts("Hello and welcome to FunOS!\n");

  vga_puts("\n");

  cpu_info info;
  char buffer_data[256];
  buffer_t buffer;

  buffer_init(&buffer, buffer_data, sizeof(buffer_data));
  cpuid_get(&info);
  cpuid_explain(&info, &buffer);
  vga_putb(&buffer);
  vga_puts("\n");

  uint8_t x = *(uint8_t *) 0x62;
  uint8_t y = *(uint16_t *) 0x63;
  print_hex8(x);
  vga_put('-');
  print_hex16(y);
  // serial_setup(serial_port1());

  // uint8_t x;
  // asm_outb(0x3fa, 0x07);
  // asm_inb(0x3fa, x);
  // hexout(x);
  // terminal_putchar(' ');
  // asm_inb(0x3f8, x);
  // hexout(x);
  // terminal_putchar(' ');
  // asm_inb(0x3fa, x);
  // hexout(x);
  //
  // asm volatile("sti");
  // terminal_putchar('%');
  // asm_inb(PIC1_DATA, x);
  // hexout(x);
  // asm_inb(PIC2_DATA, x);
  // hexout(x);
  //
  // // enable_irq(4);
  // // enable_irq(3);
  // terminal_putchar('%');
  // asm_inb(PIC1_DATA, x);
  // hexout(x);
  // asm_inb(PIC2_DATA, x);
  // hexout(x);


// uint8_t x;
// // INB(SERIAL1, x);
// // hexout(x);
// // INB(SERIAL1 + 1, x);
// // hexout(x);
//
// OUTB(SERIAL1 + 0, 0x0c);
// OUTB(SERIAL1 + 1, 0);
//
// terminal_putchar(',');
// INB(SERIAL1, x);
// hexout(x);
// INB(SERIAL1 + 1, x);
// hexout(x);
//
// OUTB(SERIAL1 + 3, MODE_DIVISOR_LATCH | MODE_8N1);

}
