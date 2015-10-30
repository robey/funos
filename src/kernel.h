#pragma once

// in %dx, %al / out %al, %dx
#define asm_inb(port, x) asm volatile("in %w1, %b0" : "=a" (x) : "d" (port))
#define asm_outb(port, x) asm volatile("out %b0, %w1" : /* no outputs */ : "a" (x), "d" (port))

// get/set MSR
#define asm_get_msr(msr, hi, lo) asm volatile("rdmsr" : "=a" (lo), "=d" (hi) : "c" (msr))
#define asm_set_msr(msr, hi, lo) asm volatile("wrmsr" : /* no outputs */ : "a" (lo), "d" (hi), "c" (msr))

// interrupts
#define asm_sti() asm volatile("sti")
#define asm_cli() asm volatile("cli")

// set the IDT
extern void _idt_init();

// use -fvisibility=hidden so most functions are un-exported.
#define export __attribute__((visibility ("default")))
//#define DLL_LOCAL  __attribute__ ((visibility ("hidden")))

#define unused __attribute__((unused))


enum {
  CPU_FAULT_DIVIDE_ZERO = 0,
  CPU_FAULT_DEBUG = 1,
  CPU_FAULT_NMI = 2,
  CPU_FAULT_BREAKPOINT = 3,
  CPU_FAULT_OVERFLOW = 4,
  CPU_FAULT_INVALID_OPCODE = 6,
  CPU_FAULT_DOUBLE_FAULT = 8,
  CPU_FAULT_GENERAL_PROTECTION = 13,
  CPU_FAULT_PAGE = 14,
  CPU_FAULT_FPU = 16,
  CPU_FAULT_SIMD = 19
} cpu_exceptions;
