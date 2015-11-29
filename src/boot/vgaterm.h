#pragma once

#include "rtc.h"

void vgaterm_add_second(void);
void vgaterm_blink(void);
void vgaterm_init(void);
void vgaterm_keyboard(uint32_t scancode);
void vgaterm_set_clock(rtc_clock *clock);
void vgaterm_write(char c);
