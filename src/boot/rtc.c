#include <stdbool.h>
#include <stdint.h>
#include "loader.h"
#include "rtc.h"

enum cmos_registers {
  RTC_SECONDS = 0,
  RTC_MINUTES = 2,
  RTC_HOURS = 4,
  RTC_DAY = 7,
  RTC_MONTH = 8,
  RTC_YEAR = 9,
  RTC_STATUS_A = 10,
  RTC_STATUS_B = 11
};

static uint8_t rtc_read(uint8_t reg) {
  uint8_t x;
  asm_outb(CMOS_ADDRESS_PORT, reg);
  asm_inb(CMOS_DATA_PORT, x);
  return x;
}

static bool rtc_busy() {
  return (rtc_read(RTC_STATUS_A) & 0x80) != 0;
}

static void rtc_fetch(rtc_clock *clock) {
  while (rtc_busy()) timer_sleep(1);
  clock->year = rtc_read(RTC_YEAR);
  clock->month = rtc_read(RTC_MONTH);
  clock->day = rtc_read(RTC_DAY);
  clock->hour = rtc_read(RTC_HOURS);
  clock->minute = rtc_read(RTC_MINUTES);
  clock->second = rtc_read(RTC_SECONDS);
}

#define FIX_BCD(n) (((n) & 0x0f) | (((n) >> 4) * 10))
void rtc_get(rtc_clock *clock) {
  rtc_clock buffer;

  while (true) {
    rtc_fetch(clock);
    rtc_fetch(&buffer);
    if (clock->year == buffer.year && clock->month == buffer.month && clock->day == buffer.day &&
        clock->hour == buffer.hour && clock->minute == buffer.minute && clock->second == buffer.second) {
      break;
    }
  }

  bool am_pm = clock->hour & 0x80;
  clock->hour &= 0x7f;

  // fix up BCD?
  uint8_t b = rtc_read(RTC_STATUS_B);
  if ((b & 4) == 0) {
    clock->year = FIX_BCD(clock->year);
    clock->month = FIX_BCD(clock->month);
    clock->day = FIX_BCD(clock->day);
    clock->hour = FIX_BCD(clock->hour);
    clock->minute = FIX_BCD(clock->minute);
    clock->second = FIX_BCD(clock->second);
  }

  // fix up 24-hour clock? :(
  if ((b & 2) == 0) clock->hour += (am_pm ? 12 : 0);
  clock->year += 2000;
}
