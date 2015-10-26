#pragma once

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
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

#define VGA_ATTR(fg, bg) ((fg) | ((bg) << 4))
#define VGA_CELL(attr, ch) ((ch) | ((attr) << 8))

void vga_clear();
void vga_color(uint8_t fg, uint8_t bg);
void vga_put(char ch);
void vga_puts(char *s);
