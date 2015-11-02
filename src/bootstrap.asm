;
; 2nd stage loader:
; this is launched by the (multiboot-compatible) bootloader.
;

%define BOOT_MAGIC          0x1badb002
%define BOOT_INFO_MAGIC     0x2badb002

%define BF_ALIGN            (1 << 0)  ; align loaded modules on page boundaries
%define BF_MEMINFO          (1 << 1)  ; provide memory map

%define BOOT_FLAGS          (BF_ALIGN | BF_MEMINFO)
%define BOOT_CHECKSUM       (-(BOOT_MAGIC + BOOT_FLAGS))

%define VGA_SCREEN_BUFFER   0xb8000
%define VGA_BOTTOM_LINE     (VGA_SCREEN_BUFFER + (2 * 80 * 24))
%define VGA_REGISTER_A      (VGA_BOTTOM_LINE + (62 * 2))
%define VGA_REGISTER_B      (VGA_BOTTOM_LINE + (71 * 2))
%define VGA_BLANK           (0x5f20)

section .multiboot
align 4

dd BOOT_MAGIC, BOOT_FLAGS, BOOT_CHECKSUM


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
  call vga_blank
  mov esi, loading_message
  call vga_display
  call vga_status_update        ; A

  ; check that we got a multiboot header
  cmp dword [ebp - 4], BOOT_INFO_MAGIC
  jne die
  call vga_status_update        ; B

  ; check cpuid for vital features
  mov eax, 0
  cpuid
  mov edi, VGA_REGISTER_B
  call vga_dump_eax

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
  call vga_status_update        ; C

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
  call vga_status_update        ; D

  ;; enable NMI.
  sti
  in al, 0x70
  and al, 0x7f
  out 0x70, al
  call vga_status_update        ; E

  extern kernel_main
;  call kernel_main
die:
  cli
  hlt

global _idt_init
_idt_init:
  ret

global _get_boot_info
_get_boot_info:
  ret



; ----- VGA screen updating

vga_blank:
  mov edi, VGA_BOTTOM_LINE
  mov ecx, 80
  mov eax, VGA_BLANK
  cld
  mov word [edi], ax
  rep stosw
  ret

; esi = string ptr
vga_display:
  mov edi, VGA_BOTTOM_LINE + 2
  mov ecx, 0
.loop:
  mov ax, [edi]
  and ax, 0xff00
  mov al, [esi + ecx]
  cmp al, 0
  je .out
  stosw
  inc ecx
  jnz .loop
.out:
  add edi, 2
  mov [vga_status_cursor], edi
  ret

vga_status_update:
  inc byte [vga_status_letter]
  mov edi, [vga_status_cursor]
  mov ax, VGA_BLANK
  and ax, 0xff00
  mov al, [vga_status_letter]
  mov [edi], ax
  ret

; display eax as hex, at edi (an address of the VGA screen)
vga_dump_eax:
  push ebx
  push ecx
  push edx
  mov ecx, 16
.loop:
  sub ecx, 2
  mov bl, al
  and bl, 15
  add bl, 0x30
  cmp bl, 0x3a
  jle .digit
  ; hex letter
  add bl, 0x61 - 0x0a
.digit:
  mov dx, [edi + ecx]
  and dx, 0xff00
  or dl, bl
  mov [edi + ecx], dx
  shr eax, 4
  cmp ecx, 0
  jne .loop
  pop edx
  pop ecx
  pop ebx
  ret

; ----- data

section .data
align 4

vga_status_cursor:
  dd 0
vga_status_letter:
  db '@'

loading_message:
  db 'Loading FunOS...', 0

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
align 4
resb 16384
stack_top:
