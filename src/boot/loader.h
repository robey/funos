#pragma once

#include <stdint.h>

#define CMOS_ADDRESS_PORT     0x70
#define CMOS_DATA_PORT        0x71

#define export __attribute__((visibility ("default")))

// in %dx, %al / out %al, %dx
#define asm_inb(port, x) asm volatile("in %w1, %b0" : "=a" (x) : "d" (port))
#define asm_outb(port, x) asm volatile("out %b0, %w1" : /* no outputs */ : "a" (x), "d" (port))

// get/set MSR
#define asm_get_msr(msr, hi, lo) asm volatile("rdmsr" : "=a" (lo), "=d" (hi) : "c" (msr))
#define asm_set_msr(msr, hi, lo) asm volatile("wrmsr" : /* no outputs */ : "a" (lo), "d" (hi), "c" (msr))

// interrupts
#define asm_sti() asm volatile("sti")
#define asm_cli() asm volatile("cli")
#define asm_disable_nmi() do { \
  uint8_t x; \
  asm_inb(CMOS_ADDRESS_PORT, x); \
  asm_outb(CMOS_ADDRESS_PORT, x | 0x80); \
} while(0)
#define asm_enable_nmi() do { \
  uint8_t x; \
  asm_inb(CMOS_ADDRESS_PORT, x); \
  asm_outb(CMOS_ADDRESS_PORT, x & 0x7f); \
} while(0)
#define asm_disable_all() do { asm_disable_nmi(); asm_cli(); } while (0)
#define asm_enable_all() do { asm_enable_nmi(); asm_sti(); } while (0)

// ----- asm calls:

extern char *vga_scrollback_buffer;
extern uint32_t vga_scrollback_size;
extern void vga_set_cursor(uint32_t offset);
extern void vga_show_cursor(void);
extern void vga_hide_cursor(void);

extern void serial_write(uint32_t data);

extern void timer_sleep(uint32_t msec);
