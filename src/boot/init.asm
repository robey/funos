;
; bootstrap:
; this is launched by the (multiboot-compatible) bootloader.
; sometimes multiboot is called a "stage-2" loader, so i guess we are at
; stage 3 now.
;

%define BOOT_MAGIC          0x1badb002
%define BOOT_INFO_MAGIC     0x2badb002

%define BF_ALIGN            (1 << 0)  ; align loaded modules on page boundaries
%define BF_MEMINFO          (1 << 1)  ; provide memory map

%define BOOT_FLAGS          (BF_ALIGN | BF_MEMINFO)
%define BOOT_CHECKSUM       (-(BOOT_MAGIC + BOOT_FLAGS))

section .multiboot
align 4

dd BOOT_MAGIC, BOOT_FLAGS, BOOT_CHECKSUM

extern crash
extern irq_init
extern serial_init, serial_write
extern vga_init, vga_status_update, vga_display_register_a, vga_display_register_b

; require: TSC, MSR, PAE, APIC, CMOV
%define CPUID_REQUIRED_EDX      0x00008270
; require: NX, LONG
%define CPUID_REQUIRED_EXT_EDX  0x20100000

;
; procedures in the bootstrap follow a special calling convention (not the
; intel one):
;   - int parameters, in order: eax, edx
;   - ptr parameters, in order: edi, esi
;   - clobbered by callee: edi, esi -- everything else must be preserved
;

section .text
global _start
_start:
  mov esp, stack_top
  mov ebp, esp
  push eax
  push ebx
  mov eax, ds
  mov es, eax

  ; display a status line showing progress
  call vga_init
  call vga_status_update        ; A

  ; check that we got a multiboot header
  cmp dword [ebp - 4], BOOT_INFO_MAGIC
  jne die
  call vga_status_update        ; B

  ; check cpuid for vital features
  mov eax, 0
  cpuid
  call vga_display_register_b
  ;; must be at least 1 attribute.
  cmp eax, 1
  jl die
  call vga_status_update        ; C
  mov eax, 0x80000000
  cpuid
  call vga_display_register_b
  ;; must be at least 1 extended attribute.
  and eax, 0x7fffffff
  cmp eax, 1
  jl die
  call vga_status_update        ; D
  ;; verify that our required features are present.
  mov eax, 1
  cpuid
  mov eax, edx
  call vga_display_register_b
  mov eax, ecx
  call vga_display_register_a
  and edx, CPUID_REQUIRED_EDX
  cmp edx, CPUID_REQUIRED_EDX
  jne die
  call vga_status_update        ; E
  mov eax, 0x80000001
  cpuid
  mov eax, edx
  call vga_display_register_b
  mov eax, ecx
  call vga_display_register_a
  and edx, CPUID_REQUIRED_EXT_EDX
  cmp edx, CPUID_REQUIRED_EXT_EDX
  jne die
  call vga_status_update        ; F

  ; enter protected mode, if we aren't already.
  ; we may not call BIOS because multiboot might have put us into protected mode already!

  ;; disable NMI.
  in al, 0x70
  or al, 0x80
  out 0x70, al
  cli

  ;; set up a new GDT that marks all of memory as code & data.
  lgdt [initial_gdt_locator]
  ;; doesn't become active until we load the segment registers.
  ;; (the first non-null entry is CS, and the second is DS/ES/FS/GS/SS.)
  jmp 0x08:.reload_cs
.reload_cs:
  mov eax, 0x10
  mov ds, eax
  mov es, eax
  mov fs, eax
  mov gs, eax
  mov ss, eax
  call vga_status_update        ; G

  ;; make sure A20 pin is active (seriously don't ask).
  in al, 0x92
  test al, 2
  jnz .no_a20
  or al, 2
  and al, 0xfe
  out 0x92, al
.no_a20:

  ;; set PE (protection enable) bit in CR0.
  mov eax, cr0
  or al, 1
  mov cr0, eax
  call vga_status_update        ; H

  ;; enable NMI.
  in al, 0x70
  and al, 0x7f
  out 0x70, al
  call vga_status_update        ; I

  call irq_init
  call vga_status_update        ; J

  call serial_init
  call vga_status_update        ; K

  sti

  mov eax, 0x40
  call serial_write
  mov eax, 0x61
  call serial_write

.wut
  nop; nop; nop
  jmp .wut
  
  jmp crash

  hlt

die:
  cli
  hlt




; ----- data

section .data
align 4

; initial GDT (global descriptor table for memory)
;
; GDT entry format appears to be:
;   LL LL BB BB BB TT xl bb
;
; L - limit (bits 0 - 15, LSB)
; B - base (bits 0 - 23, LSB)
; T - type: 0 = null, 9a = code, 92 = data
; x - format of limit:
;       4: limit is only 16 bits long
;       c: limit is bits 13 - 31, bottom 12 bits are 0xfff
; l - limit (bits 16 - 19)
; b - base (bits 24 - 31)
align 8
initial_gdt:
.gdt_entry_null:
  db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
.gdt_entry_code:
  db 0xff, 0xff, 0x00, 0x00, 0x00, 0x9a, 0xcf, 0x00
.gdt_entry_data:
  db 0xff, 0xff, 0x00, 0x00, 0x00, 0x92, 0xcf, 0x00
initial_gdt_size equ $ - initial_gdt - 1

align 8
initial_gdt_locator:
  dw initial_gdt_size
  dd initial_gdt

section .bootstrap_stack, nobits
align 4096
resb 4096
stack_top:
