#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include "loader.h"
#include "vgaterm.h"

#define LF          10
#define CR          13

#define FLAG_HEX                (1 << 0)
#define FLAG_RIGHT_JUSTIFY      (1 << 1)
#define FLAG_ZERO_PAD           (1 << 2)

static void write(char c) {
  vgaterm_write(c);
  serial_write(c);
}

static void pad(int n, int flags) {
  for (; n > 0; n--) write(flags & FLAG_ZERO_PAD ? '0' : ' ');
}

static void log_string(const char *p, int flags, int width) {
  // first, calculate the length of this string.
  uint16_t len = 0;
  for (const char *x = p; *x; x++, len++);
  if ((flags & FLAG_RIGHT_JUSTIFY) && width > len) pad(width - len, flags);
  for (const char *x = p; *x; x++) write(*x);
  if (!(flags & FLAG_RIGHT_JUSTIFY) && width > len) pad(width - len, flags);
}

static const char *HEX = "0123456789abcdef";
static void log_hex_byte(uint32_t byte) {
  write(HEX[(byte >> 4) & 0xf]);
  write(HEX[byte & 0xf]);
}

static void log_hex(uint32_t n, int bytes, int flags, int width) {
  uint16_t len = bytes * 2;
  if ((flags & FLAG_RIGHT_JUSTIFY) && width > len) pad(width - len, flags);
  if (bytes >= 4) {
    log_hex_byte(n >> 24);
    log_hex_byte(n >> 16);
  }
  if (bytes >= 2) log_hex_byte(n >> 8);
  log_hex_byte(n);
  if (!(flags & FLAG_RIGHT_JUSTIFY) && width > len) pad(width - len, flags);
}

static void log_sint(int64_t n, int flags, int width) {
  bool negative = n < 0;
  if (n < 0) n = -n;

  char buffer[24];
  char *p = &buffer[23];

  *--p = 0;
  if (n == 0) *--p = '0';
  while (n > 0) {
    *--p = (n % 10) + '0';
    n /= 10;
  }
  if (negative) *--p = '-';
  log_string(p, flags, width);
}

static void log_int(uint32_t n, int bytes, int flags, int width) {
  if (flags & FLAG_HEX) {
    log_hex(n, bytes, flags, width);
    return;
  }

  log_sint((int32_t) n, flags, width);
}

static void log_int64(uint64_t n, int flags, int width) {
  if (flags & FLAG_HEX) {
    uint16_t len = 16;
    if ((flags & FLAG_RIGHT_JUSTIFY) && width > len) pad(width - len, flags);
    log_hex((uint32_t)(n >> 32), 4, 0, 0);
    log_hex((uint32_t)n, 4, 0, 0);
    if (!(flags & FLAG_RIGHT_JUSTIFY) && width > len) pad(width - len, flags);
    return;
  }

  log_sint((int64_t) n, flags, width);
}

/*
 * format descriptors are in {...}
 * use \\{ to display a raw '{'.
 *
 * characters inside define the format of the next arg:
 *   - b/w/d/q: uint8/uint16/uint32/uint64
 *   - s: null-terminated string
 * modifiers:
 *   - x: int should be in hex
 *   - 0: number should be zero-padded
 *   - <: string should be left-justified (default)
 *   - >: string should be right-justified
 * width (follows ":"):
 *   - positive: right-justify
 *   - negative: left-justify
 *
 * example:
 *   log("Installing {s:-20} ({d} bytes) ...", packageName, bytes);
 */
void log(const char *text, ...) {
  va_list args;
  bool linefeed = true;

  // internal state for decoding format descriptors:
  char format = ' ';
  bool quoted = false;
  uint16_t flags = 0;
  int16_t width = 0;

  va_start(args, text);
  for (const char *p = text; *p; p++) {
    if (quoted) {
      write(*p);
      quoted = false;
      continue;
    }
    switch (*p) {
      case '\\':
        quoted = true;
        break;
      case '{':
        for (p++; *p && *p != '}' && *p != ':'; p++) {
          switch (*p) {
            case 's':
            case 'b':
            case 'w':
            case 'd':
            case 'q':
              format = *p;
              break;
            case 'x':
              flags |= FLAG_HEX;
              break;
            case '<':
              flags &= ~FLAG_RIGHT_JUSTIFY;
              break;
            case '>':
              flags |= FLAG_RIGHT_JUSTIFY;
              break;
            case '0':
              flags |= FLAG_ZERO_PAD | FLAG_RIGHT_JUSTIFY;
              break;
          }
        }
        if (*p == ':') {
          for (p++; *p && *p != '}'; p++) {
            if (*p >= '0' && *p <= '9') width = width * 10 + (*p - '0');
          }
        }

        switch (format) {
          case 's':
            log_string(va_arg(args, const char *), flags, width);
            break;
          case 'b':
            log_int(va_arg(args, int), 1, flags, width);
            break;
          case 'w':
            log_int(va_arg(args, int), 2, flags, width);
            break;
          case 'd':
            log_int(va_arg(args, uint32_t), 4, flags, width);
            break;
          case 'q':
            log_int64(va_arg(args, uint64_t), flags, width);
            break;
        }

        format = ' ';
        flags = 0;
        width = 0;
        break;
      default:
        write(*p);
    }
  }
  va_end(args);
  if (!quoted) {
    write(CR);
    write(LF);
  }
}
