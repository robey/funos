;
; display a pretty hex dump of registers & stack for debugging.
;

%define module crash
%include "api.macro"

section .text

; name, y, x, offset
%macro dumpreg 4
  mov edx, (%2 << 8) + %3
  call vga_compute
  mov ecx, %1
  mov eax, [reg_stack_top - %4]
  call dump_register
%endmacro

; never returns.
global crash
crash:
  mov [saved_esp], esp
  mov esp, reg_stack_top
  ; EAX, ECX, EDX, EBX, ESP (original value), EBP, ESI, and EDI:
  pushad
  pushfd
  mov eax, [saved_esp]
  mov [reg_stack_top - 20], eax

  ; blank 12 lines for crash dump
  mov eax, 12
.clear_loop:
  call vga_blank_line
  inc eax
  cmp eax, 23
  jle .clear_loop

  ; is there turkey?
  mov edx, 0x0c01
  call vga_compute
  mov ecx, 'no t'
  call vga_put_small
  mov ecx, 'urke'
  call vga_put_small
  mov ecx, 'y :('
  call vga_put_small

  ; display 8 main regs + eflags
  dumpreg ' A:', 14, 2, 4
  dumpreg ' B:', 15, 2, 16
  dumpreg ' C:', 16, 2, 8
  dumpreg ' D:', 17, 2, 12
  dumpreg 'DI:', 18, 2, 32
  dumpreg 'SI:', 19, 2, 28
  dumpreg 'BP:', 20, 2, 24
  dumpreg 'SP:', 21, 2, 20
  dumpreg ' F:', 22, 2, 36

  mov esi, [reg_stack_top - 20]
  ; round down to nearest paragraph
  and esi, 0xfffffff0
  ; draw 9 lines
  mov ecx, 0
.loop:
  mov edx, ecx
  shl edx, 8
  add edx, 0x0e14
  call vga_compute
  call dump_memory_row
  inc ecx
  cmp ecx, 9
  jl .loop

  ; display crash reason, if one was set.
  mov edx, (12 << 8) + 75
  call vga_compute
  mov ecx, [crash_reason]
  call vga_put_small

  ; die.
  cli
  hlt

; display a register (eax) named (ecx) at (edi)
dump_register:
  call vga_put_small
  add edi, 2
  call vga_dump_eax
  ret

; display a row of memory contents (esi) at (edi)
dump_memory_row:
  push eax
  push ebx
  push ecx
  mov eax, esi
  call vga_dump_eax
  mov ecx, ': '
  call vga_put_small
  mov ebx, 4
.loop:
  cmp esi, [reg_stack_top - 20]
  jne .no_highlight
  mov ecx, 8
  call vga_highlight
.no_highlight:
  mov eax, [esi]
  call vga_dump_eax
  mov ecx, ' '
  call vga_put_small
  add esi, 4
  dec ebx
  cmp ebx, 0
  jne .loop
.out:
  pop ecx
  pop ebx
  pop eax
  ret


section .bss

align 4
reg_stack:
  resb 16 * 4
reg_stack_top:

section .data

saved_esp: dd 0

global crash_reason
crash_reason: dd 0
