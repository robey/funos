%define module irq
%include "api.macro"

%define PIC1_CMD        0x0020
%define PIC1_DATA       0x0021
%define PIC2_CMD        0x00a0
%define PIC2_DATA       0x00a1

section .text

global irq_init
irq_init:
  call pic_init
  lidt [idtr]
	ret

; initialize the PIC, then disable all interrupts (to start).
pic_init:
  mov al, 0x11
  out PIC1_CMD, al
  out PIC2_CMD, al
  ; put irq ints at 0x20 - 0x2f.
  mov al, 0x20
  out PIC1_DATA, al
  mov al, 0x28
  out PIC2_DATA, al
  ; PIC1 is primary. PIC2 is secondary.
  mov al, 0x04
  out PIC1_DATA, al
  mov al, 0x02
  out PIC2_DATA, al
  ; normal view!
  mov al, 0x01
  out PIC1_DATA, al
  out PIC2_DATA, al
  ; disable all.
  mov al, 0xff
  out PIC1_DATA, al
  out PIC2_DATA, al
  ret

; eax = irq #, edi = handler
; you should probably have interrupts off. :)
;   - dw offset (low)
;   - dw selector
;   - db (zero)
;   - db flags
;   - dw offset (high)
global irq_set_handler
irq_set_handler:
  push edi
  mov [idt + (eax * 8)], di
  shr edi, 16
  mov [idt + (eax * 8) + 6], di
  mov word [idt + (eax * 8) + 2], 0x0008
  mov byte [idt + (eax * 8) + 5], 0x8e
  pop edi
  ret

; eax = irq#
global irq_enable
irq_enable:
  push ebx
  push ecx
  push edx
  ; bx = ~(1 << ax)
  xor bx, bx
  inc bx
  mov cl, al
  shl bx, cl
  not bx
  ; which port?
  mov edx, PIC1_DATA
  cmp eax, 8
  jl .ready
  mov edx, PIC2_DATA
  shr bx, 8
.ready:
  in al, dx
  and al, bl
  out dx, al
  pop edx
  pop ecx
  pop ebx
  ret

; eax = irq#
global irq_disable
irq_disable:
  push ebx
  push ecx
  push edx
  ; bx = (1 << ax)
  xor bx, bx
  inc bx
  mov cl, al
  shl bx, cl
  ; which port?
  mov edx, PIC1_DATA
  cmp eax, 8
  jl .ready
  mov edx, PIC2_DATA
  shr bx, 8
.ready:
  in al, dx
  or al, bl
  out dx, al
  pop edx
  pop ecx
  pop ebx
  ret


section .data

; the IDT is up to 256 8-byte entries. we only need 32 for CPU exceptions,
; plus the 16 base IRQs at this stage.
align 8
idt:
%rep 0x30
  dq 0
%endrep

idtr:
  dw 0x30 * 8 - 1
  dd idt
