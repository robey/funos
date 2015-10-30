#include <stdint.h>
#include "kernel.h"
#include "pic.h"

/*
 * init (if necessary) the PIC and then disable the APIC.
 * since we will be running in VMs, we don't care about multicore.
 */
void pic_init(void) {
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

  // disable APIC.
  uint32_t hi, lo;
  asm_get_msr(APIC_BASE_MSR, hi, lo);
  lo &= ~(1 << 11);
  asm_set_msr(APIC_BASE_MSR, hi, lo);
}
