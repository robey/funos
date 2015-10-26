#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "kernel.h"
#include "vga.h"

void print_hex8(uint8_t n) {
#define __hex_digit(n) ((n) < 10 ? '0' + (n) : 'a' + (n) - 10)
  vga_put(__hex_digit((n >> 4) & 0xf));
  vga_put(__hex_digit(n & 0xf));
}

void print_hex32(uint32_t n) {
  print_hex8(n >> 24);
  print_hex8(n >> 16);
  print_hex8(n >> 8);
  print_hex8(n);
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

static inline void cpuid(int code, uint32_t regs[4]) {
  asm volatile("cpuid" : "=a" (regs[0]), "=b" (regs[1]), "=c" (regs[2]), "=d" (regs[3]) : "a" (code));
}

// -----

typedef struct {
  char id[13];
  uint32_t highest_feature;
  uint32_t highest_extended_feature;
  uint32_t feature_edx;
  uint32_t feature_ecx;
} cpu_info;

void get_cpu_info(cpu_info *info) {
  uint32_t regs[4];

  cpuid(0, regs);
#define _copy(n, x) do { \
  info->id[n] = regs[x] & 0xff; \
  info->id[n + 1] = regs[x] >> 8; \
  info->id[n + 2] = regs[x] >> 16; \
  info->id[n + 3] = regs[x] >> 24; \
} while (0)
  _copy(0, 1);
  _copy(4, 3);
  _copy(8, 2);
  info->id[12] = 0;
  info->highest_feature = regs[0];

  cpuid(0x8000000, regs);
  info->highest_extended_feature = regs[0];
  print_hex32(info->highest_extended_feature);
  vga_put('-');

  cpuid(1, regs);
  info->feature_edx = regs[3];
  info->feature_ecx = regs[2];
  print_hex32(regs[3]);
  vga_put('/');
  print_hex32(regs[2]);
}

// -----


#include "terminal.h"
export void kernel_main() {
	vga_clear();
	vga_puts("Hello and welcome to FunOS!\n");

  asm volatile("int $0x21");
  asm volatile("int $0x24");

  vga_puts("\n");

  cpu_info info;
  get_cpu_info(&info);
  vga_puts(info.id);

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
