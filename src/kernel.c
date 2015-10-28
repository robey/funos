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
  vga_put(',');
  print_hex16(n);
}

export void _isr_irq(uint32_t irq, unused uint32_t regs) {
  vga_put('$');
  print_hex8(irq);
}

// init (if necessary) the PIC and then disable it.
// the PIC is superceded by APIC in modern processors.
#define PIC1_CMD  0x0020
#define PIC1_DATA 0x0021
#define PIC2_CMD  0x00a0
#define PIC2_DATA 0x00a1

void pic_init() {
  asm_outb(PIC1_CMD, 0x11);
  asm_outb(PIC2_CMD, 0x11);
  // put irq ints at 0x20 - 0x2f.
  asm_outb(PIC1_DATA, 0x20);
  asm_outb(PIC2_DATA, 0x28);
  // PIC1 is primary. PIC2 is secondary.
  asm_outb(PIC1_DATA, 0x04);
  asm_outb(PIC2_DATA, 0x02);
  // normal view.
  asm_outb(PIC1_DATA, 0x01);
  asm_outb(PIC2_DATA, 0x01);
  // disable all.
  asm_outb(PIC1_DATA, 0xff);
  asm_outb(PIC2_DATA, 0xff);
}


#include "terminal.h"
export void kernel_main() {
  pic_init();
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

  uint32_t hi, lo;
#define APIC_BASE_MSR 0x1b
  asm_get_msr(APIC_BASE_MSR, hi, lo);
  print_hex32(hi);
  vga_put('/');
  print_hex32(lo);

  vga_puts("\n");
  asm volatile("movl %%esp, %0" : "=a" (lo) : );
  print_hex32(lo);
  vga_put(' ');
  asm volatile("movl %%ss, %0" : "=a" (lo) : );
  print_hex32(lo);
//  print_hex32(*(uint32_t *) 0x00001000);

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
