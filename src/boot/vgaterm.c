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
static rtc_clock clock;

#define ROWS 24
#define COLS 80
#define SCREEN_SIZE ROWS * COLS
#define LOC(x, y) *(screen + (y) * COLS + (x))
#define SPUTC(x, c) LOC(x, 24) = (LOC(x, 24) & 0xff00) | (c)

/*
 * format of the screen is a framebuffer of 80 * 25 uint16.
 * each uint16 is: 0xBFCC
 *   - B: background color
 *   - F: foreground color
 *   - C: (ascii) char
 * we don't care about color so the color bytes will always be set to some
 * default.
 */

#define COLOR           ((uint16_t) 0x0700)
#define STATUS_COLOR    ((uint16_t) 0x5f00)
#define SPACE           0x20

#define KEY_UP          0x148
#define KEY_DOWN        0x150
#define KEY_PGUP        0x149
#define KEY_PGDN        0x151
#define KEY_ESC         0x001

#define SCROLLBAR_X     25
#define SCROLLBAR_WIDTH 45

#define BLOCK_LR        0xdb
#define BLOCK_L         0xdd
#define BLOCK_R         0xde
#define EDGE_L          0x11
#define EDGE_R          0x10

#define STATUS_DISPLAY(loc, text) do { \
  const char *p = (text); \
  int x = (loc); \
  while (*p) { \
    SPUTC(x, *p); \
    x++; \
    p++; \
  } \
} while (0)

static void vgaterm_draw_scrollback() {
  for (int y = 0; y < ROWS; y++) {
    uint16_t row = scrollback_start + (scrollback_top + y) * COLS;
    if (row >= scrollback_lines * COLS) row -= scrollback_lines * COLS;
    for (int x = 0; x < COLS; x++) {
      LOC(x, y) = COLOR | *(vga_scrollback_buffer + row + x);
    }
  }

  if (scrolling) {
    // draw a scrollbar.
    SPUTC(SCROLLBAR_X, EDGE_L);
    // buffer is [0, scrollback_max + ROWS). display port is [scrollback_top, scrollback_top + ROWS).
    float bottom = scrollback_max + ROWS;
    float start = (float) scrollback_top / bottom, end = (float) (scrollback_top + ROWS) / bottom;
    for (int x = 0; x < SCROLLBAR_WIDTH; x++) {
      float left = (float) x / (float) SCROLLBAR_WIDTH;
      float middle = ((float) x + 0.5) / (float) SCROLLBAR_WIDTH;
      float right = ((float) x + 1.0) / (float) SCROLLBAR_WIDTH;
      bool left_on = left >= start && middle < end;
      bool right_on = middle >= start && right < end;
      SPUTC(SCROLLBAR_X + x + 1, left_on ? (right_on ? BLOCK_LR : BLOCK_L) : (right_on ? BLOCK_R : ' '));
    }
    SPUTC(SCROLLBAR_X + SCROLLBAR_WIDTH + 1, EDGE_R);
  } else {
    for (int x = 0; x < SCROLLBAR_WIDTH + 2; x++) SPUTC(SCROLLBAR_X + x, ' ');
    STATUS_DISPLAY(SCROLLBAR_X, "Use \x19\x18 for scrollback");
  }
}

static void vgaterm_cr() {
  int col = (cursor - screen) % 80;
  cursor -= col;
  scrollback_cursor -= col;
}

static void vgaterm_lf() {
  vgaterm_cr();
  if (!scrolling) {
    for (uint16_t *p = screen + COLS; p < screen + SCREEN_SIZE; p++) {
      *(p - COLS) = *p;
    }
    for (uint16_t *p = screen + SCREEN_SIZE - COLS; p < screen + SCREEN_SIZE; p++) {
      *p = COLOR | SPACE;
    }
  }

  scrollback_cursor = vga_scrollback_buffer + scrollback_end;
  for (int i = 0; i < COLS; i++) *(scrollback_cursor + i) = SPACE;

  if (scrollback_end == scrollback_start) {
    scrollback_start += COLS;
    if (scrollback_start == scrollback_lines * COLS) scrollback_start = 0;
    if (scrolling) {
      scrollback_top--;
      if (scrollback_top < 0) scrollback_top = 0;
    }
  }

  scrollback_end += COLS;
  if (scrollback_end == scrollback_lines * COLS) scrollback_end = 0;
  if (scrolling) {
    scrollback_max++;
    if (scrollback_max >= scrollback_lines - ROWS) scrollback_max = scrollback_lines - ROWS;
    vgaterm_draw_scrollback();
  }

  vga_set_cursor(cursor - screen);
}

static void vgaterm_scrollback(int lines) {
  if (!scrolling) {
    // move scrollback_end to a conceptual place "greater than" scrollback_start, in case it's wrapped around.
    uint16_t end = scrollback_end;
    if (end <= scrollback_start) end += scrollback_lines * COLS;
    scrollback_top = ((end - scrollback_start) / COLS) - ROWS;
    scrollback_max = scrollback_top;
    scrolling = true;
    vga_hide_cursor();
  }
  scrollback_top += lines;
  if (scrollback_top >= scrollback_max) {
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
    case KEY_ESC:
      vgaterm_scrollback(scrollback_max - scrollback_top + 1);
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
        if (scrolling) {
          cursor++;
        } else {
          *cursor++ = COLOR | c;
        }
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

  // make a blank status bar at the bottom of the screen.
  for (int x = 0; x < COLS; x++) LOC(x, 24) = STATUS_COLOR | SPACE;
  STATUS_DISPLAY(1, "FunOS console viewer");

  cursor -= COLS;
  scrollback_cursor -= COLS;
  vgaterm_lf();
  ready = true;
}

static void vgaterm_update_status_bar(void) {
  SPUTC(78, '0' + (clock.minute % 10));
  SPUTC(77, '0' + (clock.minute / 10));
  SPUTC(76, ':');
  SPUTC(75, '0' + (clock.hour % 10));
  SPUTC(74, '0' + (clock.hour / 10));
}

void vgaterm_set_clock(rtc_clock *in) {
  // only care about these three fields.
  clock.hour = in->hour;
  clock.minute = in->minute;
  clock.second = in->second;
  vgaterm_update_status_bar();
}

void vgaterm_add_second(void) {
  clock.second++;
  if (clock.second > 59) {
    clock.second = 0;
    clock.minute++;
    if (clock.minute > 59) {
      clock.minute = 0;
      clock.hour++;
      if (clock.hour > 23) {
        clock.hour = 0;
      }
    }
  }
  vgaterm_update_status_bar();
}
