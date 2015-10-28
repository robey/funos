#pragma once

#include "buffer.h"

/* Hardware text mode color constants. */
enum vga_color {
	COLOR_BLACK = 0,
	COLOR_BLUE = 1,
	COLOR_GREEN = 2,
	COLOR_CYAN = 3,
	COLOR_RED = 4,
	COLOR_MAGENTA = 5,
	COLOR_BROWN = 6,
	COLOR_LIGHT_GREY = 7,
	COLOR_DARK_GREY = 8,
	COLOR_LIGHT_BLUE = 9,
	COLOR_LIGHT_GREEN = 10,
	COLOR_LIGHT_CYAN = 11,
	COLOR_LIGHT_RED = 12,
	COLOR_LIGHT_MAGENTA = 13,
	COLOR_YELLOW = 14,
	COLOR_WHITE = 15,
};

#define VGA_SCREEN_BUFFER 0xb8000
#define VGA_PORT_SELECT 0x3d4
#define VGA_PORT_DATA 0x3d5
#define VGA_REGISTER_CURSOR_START   0x0a
#define VGA_REGISTER_CURSOR_END     0x0b
#define VGA_REGISTER_CURSOR_HIGH    0x0e
#define VGA_REGISTER_CURSOR_LOW     0x0f

#define VGA_WIDTH 80
#define VGA_HEIGHT 25

#define VGA_ATTR(fg, bg) ((fg) | ((bg) << 4))
#define VGA_CELL(attr, ch) ((ch) | ((attr) << 8))

void vga_clear();
void vga_color(uint8_t fg, uint8_t bg);
void vga_put(char ch);
void vga_puts(const char *s);
void vga_putb(const buffer_t *b);
