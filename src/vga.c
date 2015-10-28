#include <stdint.h>
#include "kernel.h"
#include "vga.h"

/*
 * operate the VGA terminal
 */

static uint16_t *screen_buffer = (uint16_t *) VGA_SCREEN_BUFFER;
static uint8_t cursor_y = 0, cursor_x = 0;
static uint8_t attr = VGA_ATTR(COLOR_YELLOW, COLOR_GREEN);

static void set_cursor() {
  uint16_t offset = cursor_y * VGA_WIDTH + cursor_x;
  asm_outb(VGA_PORT_SELECT, VGA_REGISTER_CURSOR_LOW);
  asm_outb(VGA_PORT_DATA, offset & 0xff);
  asm_outb(VGA_PORT_SELECT, VGA_REGISTER_CURSOR_HIGH);
  asm_outb(VGA_PORT_DATA, offset >> 8);
}

void vga_clear() {
  uint16_t cell = VGA_CELL(attr, ' ');
  for (uint16_t i = 0; i < VGA_HEIGHT * VGA_WIDTH; i++) {
    screen_buffer[i] = cell;
  }
  cursor_y = cursor_x = 0;
  set_cursor();

  asm_outb(VGA_PORT_SELECT, VGA_REGISTER_CURSOR_START);
  asm_outb(VGA_PORT_DATA, 0);
  asm_outb(VGA_PORT_SELECT, VGA_REGISTER_CURSOR_END);
  asm_outb(VGA_PORT_DATA, 15);
}

void vga_color(uint8_t fg, uint8_t bg) {
  attr = VGA_ATTR(fg, bg);
}

void vga_put(char ch) {
  screen_buffer[cursor_y * VGA_WIDTH + cursor_x] = VGA_CELL(attr, ch);
  cursor_x++;
  if (cursor_x >= VGA_WIDTH) {
    cursor_x = 0;
    cursor_y++;
    if (cursor_y >= VGA_HEIGHT) {
      // FIXME scroll
      cursor_y = 0;
    }
  }
  set_cursor();
}

static void vga_put_string(const char *s, uint32_t length) {
  for (uint32_t i = 0; i < length; i++) {
    char ch = s[i];
    switch (ch) {
      case '\n':
        cursor_x = 0;
        cursor_y++;
        if (cursor_y >= VGA_HEIGHT) {
          // FIXME scroll
          cursor_y = 0;
        }
        set_cursor();
        break;
      default:
        vga_put(ch);
        break;
    }
  }
}

void vga_puts(const char *s) {
  uint32_t length = 0;
  while (s[length]) length++;
  vga_put_string(s, length);
}

void vga_putb(const buffer_t *b) {
  vga_put_string(b->buffer, b->used);
}
