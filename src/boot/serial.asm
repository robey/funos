;
; try to use the serial port (COM1) to communicate.
;

%define module serial
%include "api.macro"
%include "io.macro"

%define SERIAL1_PORT            0x3f8
%define SERIAL1_IRQ             4
%define MODE_8N1                0x03
%define MODE_DIVISOR_LATCH      0x80

%define DIVISOR                 115200
%define SPEED_9600              (DIVISOR / 9600)
%define SPEED_38400             (DIVISOR / 38400)

%define PORT_TX                 0  ; w
%define PORT_RX                 0  ; r
%define PORT_DIVISOR_LOW        0  ; rw latch
%define PORT_INT_ENABLE         1  ; rw
%define PORT_DIVISOR_HIGH       1  ; rw latch
%define PORT_INT_ID             2  ; r
%define PORT_FIFO_CONTROL       2  ; w
%define PORT_LINE_CONTROL       3  ; rw
%define PORT_MODEM_CONTROL      4  ; rw
%define PORT_LINE_STATUS        5  ; r
%define PORT_MODEM_STATUS       6  ; r
%define PORT_SCRATCH            7  ; rw

%define INT_TX                  (1 << 1)
%define INT_RX                  (1 << 0)

%define STATUS_INT              (1 << 0)
%define STATUS_REASON           (7 << 1)
%define STATUS_REASON_TX        (1 << 1)
%define STATUS_REASON_RX        (2 << 1)

%define LINE_STATUS_TX_READY    (1 << 5)

extern irq_enable, irq_set_handler
extern vga_display_register_b

section .text

; set serial port 1 to 38400, 8N1
global serial_init
serial_init:
  push eax
  push edx
  outio SERIAL1_PORT + PORT_LINE_CONTROL, MODE_DIVISOR_LATCH | MODE_8N1
  ; in theory, there could be high bits set, but anything 1200bps or better
  ; won't use them.
  outio SERIAL1_PORT + PORT_DIVISOR_HIGH, 0
  outio SERIAL1_PORT + PORT_DIVISOR_LOW, SPEED_38400
  outio SERIAL1_PORT + PORT_LINE_CONTROL, MODE_8N1

  ; turn on interrupt for receive
  outio SERIAL1_PORT + PORT_INT_ENABLE, INT_RX

  mov eax, 0x20 + SERIAL1_IRQ
  mov edi, irq_handler
  call irq_set_handler
  mov eax, SERIAL1_IRQ
  call irq_enable
  pop edx
  pop eax
  ret

irq_handler:
  push eax
  push edx
  inio SERIAL1_PORT + PORT_INT_ID
  test al, STATUS_INT
  jnz .out
  and al, STATUS_REASON
  cmp al, STATUS_REASON_RX
  jne .out
  xor eax, eax
  inio SERIAL1_PORT + PORT_RX
  call vga_display_register_b
  call serial_write
.out:
  outio 0x20, 0x20
  pop edx
  pop eax
  iret

; write al
global serial_write
serial_write:
  push edx
  push eax
.loop:
  inio SERIAL1_PORT + PORT_LINE_STATUS
  test al, LINE_STATUS_TX_READY
  jz .loop
  pop eax
  outioa SERIAL1_PORT + PORT_TX
  pop edx
  ret
