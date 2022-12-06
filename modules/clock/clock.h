// Copyright 2022 Matías Charrut
// This code is licensed under MIT license (see LICENSE for details)

#ifndef MODULES_CLOCK_CLOCK_H_
#define MODULES_CLOCK_CLOCK_H_

#include <string>
#include "mbed.h"
#include "wifi.h"
#include "Json.h"

#define BUFFER_SIZE_TIME_24 8
#define BUFFER_SIZE_TIME_12 11
#define BUFFER_SIZE_DATE 10

#define DAYS_OF_WEEK {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", \
                      "Friday", "Saturday"}

// ver despues como hacer con los dias de la semana. Los guardo con texto,
// lo cambio en el constructor, en el llamado de sync, etc.

struct Time {
  const uint8_t seconds;
  const uint8_t minutes;
  const uint8_t hours;
  const bool pm;
  const bool mode_twelve;
  const uint8_t day;
  const uint8_t date;
  const uint8_t month;
  const uint8_t year;

  Time(uint8_t seconds = 0, uint8_t minutes = 0, uint8_t hours = 0,
       bool pm = false, bool mode_twelve = false, uint8_t day = 0,
       uint8_t date = 0, uint8_t month = 0, uint8_t year = 0);

  Time() = delete;

  std::string formatTime() const;
  std::string formatDate() const;
  std::string formatDayWeek() const;

  static std::string dayNumberToDayWord(uint8_t);
  static uint8_t dayWordToDayNumber(const std::string &);

  // ver como eliminar constructor de struct por {}
};

class Clock {
 private:
  I2C rtc;
  uint8_t address;

  static uint8_t bcd2dec(uint8_t);
  static uint8_t dec2bcd(uint8_t);

 public:
  Clock(PinName, PinName, uint8_t);
  bool set(const Time &);
  Time get() const;
  bool sync(WiFi &, const std::string &);
};

#endif  // MODULES_CLOCK_CLOCK_H_
