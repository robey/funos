%define PIC1_CMD        0x0020
%define PIC1_DATA       0x0021
%define PIC2_CMD        0x00a0
%define PIC2_DATA       0x00a1
%define APIC_BASE_MSR   0x1b

extern isr_cpu, isr_irq
extern vga_display_register_a, vga_display_register_b

global apic_init

section .data
align 4

apic_base_address dd 0

; ----- macros for defining IDT entries
%macro idt_entry 0
  ; selector 0x08, no offset (yet), flags = 0x8e
  dw 0x0000  ; offset (low)
  dw 0x0008  ; selector
  db 0x00
  db 0x8e    ; flags
  dw 0x0000  ; offset (high)
%endmacro
%macro idt_zero 0
  ; same, but all zero'd out.
  dd 0x00000000
  dd 0x00000000
%endmacro

align 8
global idt
idt:
; 20 cpu exceptions and 12 unused
%rep 20
  idt_entry
%endrep
%rep 12
  idt_zero
%endrep
; 16 remapped IRQs
%rep 16
  idt_entry
%endrep
; 208 unused
%rep 208
  idt_zero
%endrep

idtr:
  dw 0x7ff
  dd idt




section .text

apic_init:
  call pic_init
  ; where is the apic base address?

  ; what's in your belly?!
  mov ecx, APIC_BASE_MSR
  rdmsr
  mov ebx, eax
  and ebx, 0xfffff000
  mov [apic_base_address], ebx

  call idt_init

  call vga_display_register_b
  mov eax, edx
  call vga_display_register_a
  ret

; initialize, and then disable, the old PIC.
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

%macro setgate 2
  ; setgate (int#) (addr)
  mov eax, %2
  mov [idt + (%1 << 3)], ax
  shr eax, 16
  mov [idt + (%1 << 3) + 6], ax
%endmacro

idt_init:
	setgate 0x00, _isr_cpu00
	setgate 0x01, _isr_cpu01
	setgate 0x02, _isr_cpu02
	setgate 0x03, _isr_cpu03
	setgate 0x04, _isr_cpu04
	setgate 0x05, _isr_cpu05
	setgate 0x06, _isr_cpu06
	setgate 0x07, _isr_cpu07
	setgate 0x08, _isr_cpu08
	setgate 0x09, _isr_cpu09
	setgate 0x0A, _isr_cpu0A
	setgate 0x0B, _isr_cpu0B
	setgate 0x0C, _isr_cpu0C
	setgate 0x0D, _isr_cpu0D
	setgate 0x0E, _isr_cpu0E
	setgate 0x0F, _isr_cpu0F
	setgate 0x10, _isr_cpu10
	setgate 0x11, _isr_cpu11
	setgate 0x12, _isr_cpu12
	setgate 0x13, _isr_cpu13
	setgate 0x20, _isr_irq00
	setgate 0x21, _isr_irq01
	setgate 0x22, _isr_irq02
	setgate 0x23, _isr_irq03
	setgate 0x24, _isr_irq04
	setgate 0x25, _isr_irq05
	setgate 0x26, _isr_irq06
	setgate 0x27, _isr_irq07
	setgate 0x28, _isr_irq08
	setgate 0x29, _isr_irq09
	setgate 0x2A, _isr_irq0A
	setgate 0x2B, _isr_irq0B
	setgate 0x2C, _isr_irq0C
	setgate 0x2D, _isr_irq0D
	setgate 0x2E, _isr_irq0E
	setgate 0x2F, _isr_irq0F
	lidt [idtr]
	ret

%macro isr_cpu 1
  pusha
  push %1
  jmp common_exception
%endmacro

%macro push0_isr_cpu 1
  pusha
  isr_cpu %1
%endmacro

%macro isr_irq_pic1 1
  push 0
  pusha
  mov al, %1
  jmp common_irq_pic1
%endmacro

%macro isr_irq_pic2 1
  push 0
  pusha
  mov al, %1
  jmp common_irq_pic2
%endmacro


; ISR stubs for the IDT gates we'll use
_isr_cpu00: push0_isr_cpu 0x00
_isr_cpu01: push0_isr_cpu 0x01
_isr_cpu02: push0_isr_cpu 0x02
_isr_cpu03: push0_isr_cpu 0x03
_isr_cpu04: push0_isr_cpu 0x04
_isr_cpu05: push0_isr_cpu 0x05
_isr_cpu06: push0_isr_cpu 0x06
_isr_cpu07: push0_isr_cpu 0x07
_isr_cpu08:       isr_cpu 0x08
_isr_cpu09: push0_isr_cpu 0x09
_isr_cpu0A:       isr_cpu 0x0A
_isr_cpu0B:       isr_cpu 0x0B
_isr_cpu0C:       isr_cpu 0x0C
_isr_cpu0D:       isr_cpu 0x0D
_isr_cpu0E:       isr_cpu 0x0E
_isr_cpu0F: push0_isr_cpu 0x0F
_isr_cpu10: push0_isr_cpu 0x10
_isr_cpu11:       isr_cpu 0x11
_isr_cpu12: push0_isr_cpu 0x12
_isr_cpu13: push0_isr_cpu 0x13
_isr_irq00:  isr_irq_pic1 0x00
_isr_irq01:  isr_irq_pic1 0x01
_isr_irq02:  isr_irq_pic1 0x02
_isr_irq03:  isr_irq_pic1 0x03
_isr_irq04:  isr_irq_pic1 0x04
_isr_irq05:  isr_irq_pic1 0x05
_isr_irq06:  isr_irq_pic1 0x06
_isr_irq07:  isr_irq_pic1 0x07
_isr_irq08:  isr_irq_pic2 0x08
_isr_irq09:  isr_irq_pic2 0x09
_isr_irq0A:  isr_irq_pic2 0x0A
_isr_irq0B:  isr_irq_pic2 0x0B
_isr_irq0C:  isr_irq_pic2 0x0C
_isr_irq0D:  isr_irq_pic2 0x0D
_isr_irq0E:  isr_irq_pic2 0x0E
_isr_irq0F:  isr_irq_pic2 0x0F

common_exception:
  cld
  pop eax
  push esp
  push eax
  call isr_cpu
  add esp, 8
  popa
  add esp, 4
  sti
  iret

common_irq_pic1:
	cld
  push esp
  and eax, 0xf
  push eax
  mov al, 0x20
  out 0x20, al
  call isr_irq
  add esp, 8
  popa
  add esp, 4
  sti
  iret

common_irq_pic2:
	cld
  push esp
  and eax, 0xf
  push eax
  mov al, 0x20
  out 0xa0, al
  out 0x20, al
  call isr_irq
  add esp, 8
  popa
  add esp, 4
  sti
  iret
