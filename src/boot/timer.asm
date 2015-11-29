;
; the PIT runs at 5 * 7 * 9 / (3 * 8 * 11) Mhz, which is about 1.193182 Mhz.
; we can get a rough approximation of a 1ms timer by dividing by 1193, and
; that'll be good enough for anything that isn't realtime.
;

%define module timer
%include "api.macro"
%include "io.macro"

%define TIMER_IRQ             0
%define RTC_IRQ               8
%define TIMER0_PORT           0x40
%define TIMER_COMMAND_PORT    0x43
; 00 (timer 0), 11 (set by sending low then high), 010 (rate generator), 0 (binary):
%define TIMER_RATE_GENERATOR  0x34
%define TIMER_FREQUENCY       1193
%define SECOND_IN_MSEC        1000

section .text

global timer_init
timer_init:
  push eax
  push edx
  outio TIMER_COMMAND_PORT, TIMER_RATE_GENERATOR
  outio TIMER0_PORT, TIMER_FREQUENCY & 0xff
  outio TIMER0_PORT, TIMER_FREQUENCY >> 8
  mov eax, 0x20 + TIMER_IRQ
  mov edi, timer_handler
  call irq_set_handler
  mov eax, TIMER_IRQ
  call irq_enable
  ; cargo cult: this seems to make the RTC behave more reliably under qemu.
  ; even tho we don't set a handler!
  mov eax, RTC_IRQ
  call irq_enable
  pop edx
  pop eax
  ret

timer_handler:
  push eax
  push edx
  inc dword [timer_ticks]
  inc dword [timer_msec]
  mov eax, [timer_msec]
  cmp eax, SECOND_IN_MSEC
  jne .out
  mov dword [timer_msec], 0
  call event_second_timer
.out:
  outio 0x20, 0x20
  pop edx
  pop eax
  iret

; (external) wait for N ticks to go by.
global timer_sleep
timer_sleep:
  push ebp
  mov ebp, esp
  push eax
  mov eax, [timer_ticks]
  add eax, [ebp + 8]
.loop:
  hlt
  cmp eax, [timer_ticks]
  jg .loop
.out:
  mov esp, ebp
  pop ebp
  ret

section .data

timer_ticks:
  dd 0
timer_msec:
  dd 0
