;
; notice when a key is pressed, and call a C handler.
;

%define module keyboard
%include "api.macro"

%define KEYBOARD_IRQ 1
%define KEYBOARD_DATA_PORT      0x60
%define KEYBOARD_SET_SCANCODE   0xf0

global keyboard_init
;extern event_keyboard
extern irq_enable, irq_set_handler

section .text

keyboard_init:
  push eax
  push edx
  ; choose scan code set 2
  ; (does not work.)
;  mov dx, KEYBOARD_DATA_PORT
;  mov al, KEYBOARD_SET_SCANCODE
;  out dx, al
;  mov al, 2
;  out dx, al
  mov eax, 0x20 + KEYBOARD_IRQ
  mov edi, keyboard_handler
  call irq_set_handler
  mov eax, KEYBOARD_IRQ
  call irq_enable
  pop edx
  pop eax
  ret

keyboard_handler:
  push eax
  push ebx
  push edx
  xor eax, eax
  mov dx, KEYBOARD_DATA_PORT
  in al, dx
  ; e0, f0 are modifier flags so it can represent more than 256 events.
  ; we turn them into bits 8 and 9.
  mov bx, 0x100
  cmp ax, 0xe0
  cmove ax, bx
  je .flags
  shl bx, 1
  cmp ax, 0xf0
  cmove ax, bx
  je .flags
  or ax, [keyboard_buffer]
  ;
  extern vga_display_register_b
  call vga_display_register_b
  ;
  mov word [keyboard_buffer], 0
  xor ax, ax
.flags:
  or [keyboard_buffer], ax
.out:
  mov dx, 0x20
  mov al, 0x20
  out dx, al
  pop edx
  pop ebx
  pop eax
  iret

section .data

keyboard_buffer:
  dw 0
