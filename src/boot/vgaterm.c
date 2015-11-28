#include <stdbool.h>
#include <stdint.h>
#include "loader.h"
#include "vgaterm.h"

/*
 * simple terminal viewer for the VGA screen that "may" be attached to our
 * virtualized machine.
 *
 * this thing is *really* simple. it is for viewing only. it allows pgup/
 * pgdn for scrolling, and that's it.
 *
 * a 16KB buffer gives it about 200 lines of scrollback.
 */

static uint16_t *screen = (uint16_t *)0xb8000;
static char *scrollback_cursor;
static uint16_t *cursor;

// max lines available in scrollback, calculated from the buffer size.
static uint16_t scrollback_lines;
// offset of the beginning of scrollback (line) and the end (line, exclusive).
// eventually, end will be the same as start.
static uint16_t scrollback_start, scrollback_end;
// line # from scrollback_start.
static int16_t scrollback_top, scrollback_max;

static bool ready = false, scrolling = false;

#define ROWS 24
#define COLS 80
#define SCREEN_SIZE ROWS * COLS
#define LOC(x, y) *(screen + y * COLS + x)

/*
 * format of the screen is a framebuffer of 80 * 25 uint16.
 * each uint16 is: 0xBFCC
 *   - B: background color
 *   - F: foreground color
 *   - C: (ascii) char
 * we don't care about color so the color bytes will always be set to some
 * default.
 */

#define COLOR ((uint16_t) 0x0700)
#define SPACE 0x20

#define KEY_UP      0x148
#define KEY_DOWN    0x150
#define KEY_PGUP    0x149
#define KEY_PGDN    0x151
#define KEY_ESC     0x001

static void vgaterm_draw_scrollback() {
  for (int y = 0; y < ROWS; y++) {
    uint16_t row = scrollback_start + (scrollback_top + y) * COLS;
    if (row > scrollback_lines * COLS) row -= scrollback_lines * COLS;
    for (int x = 0; x < COLS; x++) {
      LOC(x, y) = COLOR | *(vga_scrollback_buffer + row + x);
    }
  }
}

static void vgaterm_cr() {
  int col = (cursor - screen) % 80;
  cursor -= col;
  scrollback_cursor -= col;
}

static void vgaterm_lf() {
  vgaterm_cr();
  for (uint16_t *p = screen + COLS; p < screen + SCREEN_SIZE; p++) {
    *(p - COLS) = *p;
  }
  for (uint16_t *p = screen + SCREEN_SIZE - COLS; p < screen + SCREEN_SIZE; p++) {
    *p = COLOR | SPACE;
  }
  scrollback_cursor += COLS;

  if (scrollback_end == scrollback_start) {
    scrollback_start += COLS;
    if (scrolling) {
      scrollback_top--;
      if (scrollback_top < 0) scrollback_top = 0;
      vgaterm_draw_scrollback();
    }
    if (scrollback_start == scrollback_lines * COLS) scrollback_start = 0;
  }
  for (int i = 0; i < COLS; i++) *(vga_scrollback_buffer + scrollback_end + i) = SPACE;
  scrollback_end += COLS;
  if (scrollback_end == scrollback_lines * COLS) scrollback_end = 0;

  vga_set_cursor(cursor - screen);
}

// static void vgaterm_show_scrollback(char *scrollback) {
//   for (uint16_t *p = screen; p < screen + SCREEN_SIZE; p++) *p = COLOR | *scrollback++;
// }

static void vgaterm_scrollback(int lines) {
  if (!scrolling) {
    scrollback_top = ((scrollback_end - scrollback_start) / COLS) - ROWS;
    scrollback_max = scrollback_top;
    scrolling = true;
    vga_hide_cursor();
  }
  scrollback_top += lines;
  if (scrollback_top > scrollback_max) {
    scrollback_top = scrollback_max;
    scrolling = false;
    vga_show_cursor();
  }
  if (scrollback_top < 0) scrollback_top = 0;
  vgaterm_draw_scrollback();
}

void vgaterm_keyboard(uint32_t scancode) {
  if (!ready) return;
  switch (scancode) {
    case KEY_UP:
      vgaterm_scrollback(-1);
      break;
    case KEY_DOWN:
      vgaterm_scrollback(1);
      break;
    case KEY_PGUP:
      vgaterm_scrollback(-ROWS);
      break;
    case KEY_PGDN:
      vgaterm_scrollback(ROWS);
      break;
  }
}

void vgaterm_write(char c) {
  switch (c) {
    case 10:
      vgaterm_lf();
      return;
    default:
      if (c >= 32) {
        *scrollback_cursor++ = c;
        *cursor++ = COLOR | c;
        if ((cursor - screen) % 80 == 0) vgaterm_lf();
        vga_set_cursor(cursor - screen);
      }
      break;
  }
}

void vgaterm_init(void) {
  scrollback_cursor = vga_scrollback_buffer;
  scrollback_lines = vga_scrollback_size / COLS;

  // first, copy whatever's on the screen into the buffer.
  cursor = screen;
  for (int y = 0; y < ROWS; y++) {
    for (int x = 0; x < COLS; x++) {
      *scrollback_cursor++ = (*cursor++) & 0xff;
    }
  }
  scrollback_start = 0;
  scrollback_end = ROWS * COLS;

  cursor -= COLS;
  scrollback_cursor -= COLS;
  vgaterm_lf();
  ready = true;
}

void vgaterm_blink() {
  if ((LOC(1, 24) & 0xff) == ':') {
    LOC(1, 24) = (LOC(1, 24) & 0xff00) | ' ';
  } else {
    LOC(1, 24) = (LOC(1, 24) & 0xff00) | ':';
  }
}
