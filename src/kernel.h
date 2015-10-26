#pragma once

// in %dx, %al / out %al, %dx
#define asm_inb(port, x) asm volatile("in %w1, %b0" : "=a" (x) : "d" (port))
#define asm_outb(port, x) asm volatile("out %b0, %w1" : /* no outputs */ : "a" (x), "d" (port))

// use -fvisibility=hidden so most functions are un-exported.
#define export __attribute__((visibility ("default")))
//#define DLL_LOCAL  __attribute__ ((visibility ("hidden")))

#define unused __attribute__((unused))
