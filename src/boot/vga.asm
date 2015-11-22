;
; simple routines for drawing a status line during early boot.
;

%define VGA_SCREEN_BUFFER           0xb8000
%define VGA_BOTTOM_LINE             (VGA_SCREEN_BUFFER + (2 * 80 * 24))
%define VGA_REGISTER_A              (VGA_BOTTOM_LINE + (62 * 2))
%define VGA_REGISTER_B              (VGA_BOTTOM_LINE + (71 * 2))
%define VGA_BLANK                   (0x5f20)
%define VGA_HIGHLIGHT               (0x5e)

%define VGA_SCROLLBACK_SIZE         16384

%define VGA_PORT_SELECT             0x3d4
%define VGA_PORT_DATA               0x3d5
%define VGA_REGISTER_CURSOR_START   0x0a
%define VGA_REGISTER_CURSOR_END     0x0b
%define VGA_REGISTER_CURSOR_HIGH    0x0e
%define VGA_REGISTER_CURSOR_LOW     0x0f

global \
  vga_blank_line, \
  vga_compute, \
  vga_display_register_a, \
  vga_display_register_b, \
  vga_dump_eax, \
  vga_highlight, \
  vga_init, \
  vga_put_small, \
  vga_scrollback_buffer, \
  vga_scrollback_size, \
  vga_set_cursor, \
  vga_status_update

section .text

vga_init:
  ; set big blocky cursor.
;asm_outb(VGA_PORT_SELECT, VGA_REGISTER_CURSOR_START);
;asm_outb(VGA_PORT_DATA, 0);
;asm_outb(VGA_PORT_SELECT, VGA_REGISTER_CURSOR_END);
;asm_outb(VGA_PORT_DATA, 15);

  call vga_blank_status_line
  mov esi, loading_message
  call vga_display
  ret

vga_blank_status_line:
  push eax
  mov eax, 24
  call vga_blank_line
  pop eax
  ret

; eax = line #
vga_blank_line:
  push eax
  push ecx
  push edx
  mov edx, eax
  shl edx, 8
  call vga_compute
  ; let's start blankin'.
  mov ecx, 80
  mov eax, VGA_BLANK
  cld
  rep stosw
  pop edx
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

; edi = where in vga buffer (in/out)
; ecx = LSB string of up to 4 bytes to write.
vga_put_small:
  push eax
  push ecx
.loop:
  cmp cl, 0
  je .out
  mov ax, [edi]
  mov al, cl
  stosw
  shr ecx, 8
  jnz .loop
.out:
  pop ecx
  pop eax
  ret

; highlight some chars.
; edi = where in vga buffer
; ecx = count
vga_highlight:
  push eax
  push edi
.loop:
  mov ax, [edi]
  mov ah, VGA_HIGHLIGHT
  stosw
  dec ecx
  jnz .loop
  pop edi
  pop eax
  ret

; edx YYXX -> edi
vga_compute:
  push eax
  push ecx
  mov eax, edx
  and eax, 0xff00
  ; mulitply Y by 160 by shift magic (it starts out as Y * 256).
  shr eax, 1
  mov ecx, eax
  shr eax, 2
  add ecx, eax
  mov edi, VGA_SCREEN_BUFFER
  add edi, ecx
  ; add in X
  and edx, 0xff
  shl edx, 1
  add edi, edx
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
  add edi, 16
  pop edx
  pop ecx
  pop ebx
  pop eax
  ret

; display eax at the "register A" field of the status line.
vga_display_register_a:
  mov edi, VGA_REGISTER_A
  jmp vga_dump_eax

; display eax at the "register B" field of the status line.
vga_display_register_b:
  mov edi, VGA_REGISTER_B
  jmp vga_dump_eax

; (external) put blinking cursor at linear offset from (0, 0)
vga_set_cursor:
  push ebp
  mov ebp, esp
  push eax
  push edx
  mov dx, VGA_PORT_SELECT
  mov al, VGA_REGISTER_CURSOR_LOW
  out dx, al
  mov dx, VGA_PORT_DATA
  mov eax, [ebp + 8]
  out dx, al
  mov dx, VGA_PORT_SELECT
  mov al, VGA_REGISTER_CURSOR_HIGH
  out dx, al
  mov dx, VGA_PORT_DATA
  mov eax, [ebp + 8]
  shr eax, 8
  out dx, al
  pop edx
  pop eax
  pop ebp
  ret

section .data
align 4

vga_status_cursor:
  dd 0
vga_status_letter:
  db '@'

loading_message:
  db 'Loading FunOS...', 0

vga_scrollback_size:
  dd VGA_SCROLLBACK_SIZE


; for the (later) terminal phase, allocate a 16KB scrollback buffer
section .bss
align 4096
vga_scrollback_buffer:
  resb VGA_SCROLLBACK_SIZE
