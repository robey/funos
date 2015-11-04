;
; simple routines for drawing a status line
;

%define VGA_SCREEN_BUFFER   0xb8000
%define VGA_BOTTOM_LINE     (VGA_SCREEN_BUFFER + (2 * 80 * 24))
%define VGA_REGISTER_A      (VGA_BOTTOM_LINE + (62 * 2))
%define VGA_REGISTER_B      (VGA_BOTTOM_LINE + (71 * 2))
%define VGA_BLANK           (0x5f20)

global \
  vga_init, \
  vga_status_update, \
  vga_display_register_a, \
  vga_display_register_b

section .text

vga_init:
  call vga_blank_status_line
  mov esi, loading_message
  call vga_display
  ret

vga_blank_status_line:
  push eax
  push ecx
  mov edi, VGA_BOTTOM_LINE
  mov ecx, 80
  mov eax, VGA_BLANK
  cld
  mov word [edi], ax
  rep stosw
  pop ecx
  pop eax
  ret

; esi = string ptr
vga_display:
  push eax
  push ecx
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
  pop ecx
  pop eax
  ret

; update the current status letter and display it.
; this goes (A -> B -> C -> ...) as the boot progresses, so that if it dies,
; you can see roughly where it happened.
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
  push eax
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
  jl .digit
  ; hex letter
  add bl, 0x61 - 0x3a
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
  pop eax
  ret

vga_display_register_a:
  mov edi, VGA_REGISTER_A
  jmp vga_dump_eax

vga_display_register_b:
  mov edi, VGA_REGISTER_B
  jmp vga_dump_eax


section .data
align 4

vga_status_cursor:
  dd 0
vga_status_letter:
  db '@'

loading_message:
  db 'Loading FunOS...', 0
