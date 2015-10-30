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

void print_hex64(uint64_t n) {
  print_hex32(n >> 32);
  vga_put('_');
  print_hex32(n);
}

void _isr_cpu(uint32_t id, unused uint32_t *state) {
  vga_put('$');
  print_hex8(id);
}

export void _isr_irq(uint32_t irq, unused uint32_t regs) {
  if (irq == 1) {
    uint8_t key;
    asm_inb(0x60, key);
    vga_put('@');
    print_hex8(key);
    return;
  }

  vga_put('%');
  print_hex8(irq);
}

// ----- IDT

/*
// 0x00 - 0x13: cpu exceptions
// 0x14 - 0x1f: unused
// 0x20 - 0x2f: IRQs 0 - 15
static uint64_t idt_buffer[0x30];

#define IDT_TYPE_INT_GATE 0x8e

static void idt_set_handler(int n, void *addr, uint16_t selector, uint8_t type) {
  uint16_t low = ((uint32_t) addr) & 0xffff;
  uint16_t high = ((uint32_t) addr) >> 16;
  uint8_t *desc = (uint8_t *) &idt_buffer[n];
*/

  /*
   * offset low (16)
   * selector (16)
   * unused (8)
   * type (8)
   * offset high (16)
   */
   /*
  *(uint16_t *)(desc + 0) = low;
  *(uint16_t *)(desc + 2) = selector;
  *(desc + 5) = type;
  *(uint16_t *)(desc + 6) = high;
}

static void idt_default_handler() {

}

void init_idt() {
  for (uint32_t i = 0; i < 0x30; i++) idt_buffer[i] = 0;

}
*/


// -----

#include "terminal.h"
#include "boot_info.h"
#include "irq.h"
#include "pic.h"
export void kernel_main() {
  asm_cli();
  pic_init();
  _idt_init();
  asm_sti();

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

  vga_puts("\nbootinfo: ");
  boot_info *bi = _get_boot_info();
  print_hex32(bi->flags);
  vga_puts("\n");
  if (bi->flags & BFI_MEMORY_MAP) {
    print_hex32(bi->mmap_length);
    vga_puts("\n");

    for (uint32_t i = 0; i < 10; i++) {
      print_hex32(bi->mmap_addr[i].size);
      vga_put(' ');
      print_hex64(bi->mmap_addr[i].base_addr);
      vga_put(' ');
      print_hex64(bi->mmap_addr[i].length);
      vga_put(' ');
      print_hex32(bi->mmap_addr[i].type);

      vga_puts("\n");
    }
  }

  // Send the command byte.
  // asm_outb(0x43, 0x36);
  // asm_outb(0x40, 0xff);
  // asm_outb(0x40, 0xff);

  serial_setup(serial_port1());
  irq_enable(IRQ_KEYBOARD);
  irq_enable(IRQ_SERIAL1);
  while (1) {
    asm volatile("nop");
  }

  // print_hex32(boot_info[0]);
  // print_hex32(boot_info[1]);
  // vga_put(' ');
  // print_hex32(boot_info[2]);

//  print_hex32(*(uint32_t *) 0x00001000);


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
