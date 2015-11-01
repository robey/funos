# pragma once

typedef void (*serial_handler_t)(uint8_t);

#define SERIAL_HANDLER_NONE ((serial_handler_t) 0)

uint16_t serial1_port();
uint8_t serial1_irq();
void serial_setup(uint16_t port, uint8_t irq, serial_handler_t handler);
void serial_write(uint8_t data);
