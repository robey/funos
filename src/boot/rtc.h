#pragma once

typedef struct {
  uint16_t year;
  uint8_t day, month;
  uint8_t hour, minute, second;
} rtc_clock;

void rtc_get(rtc_clock *clock);
