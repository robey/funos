# Declare constants used for creating a multiboot header.
.set ALIGN,    1<<0             # align loaded modules on page boundaries
.set MEMINFO,  1<<1             # provide memory map
.set FLAGS,    ALIGN | MEMINFO  # this is the Multiboot 'flag' field
.set MAGIC,    0x1BADB002       # 'magic number' lets bootloader find the header
.set CHECKSUM, -(MAGIC + FLAGS) # checksum of above, to prove we are multiboot

# Declare a header as in the Multiboot Standard. We put this into a special
# section so we can force the header to be in the start of the final program.
# You don't need to understand all these details as it is just magic values that
# is documented in the multiboot standard. The bootloader will search for this
# magic sequence and recognize us as a multiboot kernel.
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

# Currently the stack pointer register (esp) points at anything and using it may
# cause massive harm. Instead, we'll provide our own stack. We will allocate
# room for a small temporary stack by creating a symbol at the bottom of it,
# then allocating 16384 bytes for it, and finally creating a symbol at the top.
.section .bootstrap_stack, "aw", @nobits
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

.section .text
.global _start
.type _start, @function
_start:
  # enter protected mode, if we aren't already.
  # we may not call BIOS because multiboot might have put us into protected mode already!

  ## disable NMI.
  in $0x70, %al
  or $0x80, %al
  out %al, $0x70
  cli

  ## set up a new GDT that marks all of memory as code & data.
  lea (initial_gdt), %eax
  mov $initial_gdt_size, %dx
  call _set_gdt

  ## make sure A20 pin is active (seriously don't ask).
  in $0x92, %al
  test $2, %al
  jnz no_a20
  or $2, %al
  and $0xfe, %al
  out %al, $0x92
no_a20:

  ## set PE (protection enable) bit in CR0.
  mov %cr0, %eax
  or %al, 1
  mov %eax, %cr0

  ## enable NMI.
  sti
  in $0x70, %al
  and $0x7f, %al
  out %al, $0x70

  # point to our 16K stack for starters.
  movl $stack_top, %esp


  #call _idt_init
  #call _pic_init

  # jump to C code.
  call kernel_main

  # In case the function returns, we'll want to put the computer into an
  # infinite loop. To do that, we use the clear interrupt ('cli') instruction
  # to disable interrupts, the halt instruction ('hlt') to stop the CPU until
  # the next interrupt arrives, and jumping to the halt instruction if it ever
  # continues execution, just to be safe. We will create a local label rather
  # than real symbol and jump to there endlessly.
  cli
  hlt
.Lhang:
  jmp .Lhang

# Set the size of the _start symbol to the current location '.' minus its start.
# This is useful when debugging or when you implement call tracing.
.size _start, . - _start


# expects GDT address in %eax, size in %edx
.global _set_gdt
.type _set_gdt, @function
_set_gdt:
  mov $gdt_register, %ebx
  mov %dx, (%ebx)
  movl %eax, 2(%ebx)
  lgdt gdt_register

  # doesn't become active until we load the segment registers.
  # (assume the first non-null entry is CS, and the second is DS/ES/FS/GS/SS.)
  # you don't want to know how hard it was to figure out the at&t syntax for this jump.
  ljmp $0x08, $.reload_cs
.reload_cs:
  mov $0x10, %eax
  mov %eax, %ds
  mov %eax, %es
  mov %eax, %fs
  mov %eax, %gs
  mov %eax, %ss
  ret


.section .data

gdt_register:
  .hword 0 # size
  .word 0  # base

.section .data
.align 8

# initial GDT (global descriptor table for memory)
#
# GDT entry format appears to be:
#   LL LL BB BB BB TT xl bb
#
# L - limit (bits 0 - 15, LSB)
# B - base (bits 0 - 23, LSB)
# T - type: 0 = null, 9a = code, 92 = data
# x - format of limit:
#       4: limit is only 16 bits long
#       c: limit is bits 13 - 31, bottom 12 bits are 0xfff
# l - limit (bits 16 - 19)
# b - base (bits 24 - 31)
initial_gdt:
gdt_entry_null:
  .byte 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
gdt_entry_code:
  .byte 0xff, 0xff, 0x00, 0x00, 0x00, 0x9a, 0xcf, 0x00
gdt_entry_data:
  .byte 0xff, 0xff, 0x00, 0x00, 0x00, 0x92, 0xcf, 0x00
.set initial_gdt_size, . - initial_gdt - 1
