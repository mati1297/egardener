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

#define MAX_DAYS_SUM 30
#define MAX_HOURS_SUM 23
#define MAX_MINUTES_SUM 59
#define MAX_SECONDS_SUM 59

#define MONTH_DAYS {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}

// Represents hour and date
struct Time {
 private:
  uint8_t seconds;
  uint8_t minutes;
  uint8_t hours;
  uint8_t day;
  uint8_t month;
  uint8_t year;

 public:
  Time(uint8_t seconds = 0, uint8_t minutes = 0, uint8_t hours = 0,
       uint8_t day = 0, uint8_t month = 0, uint8_t year = 0);

  Time(const Time&);

  Time& operator=(const Time&);
  bool operator==(const Time&) const;
  bool operator<(const Time&) const;
  bool operator<=(const Time&) const;
  bool operator>(const Time&) const;
  bool operator>=(const Time&) const;

  // Add seconds
  bool addSeconds(uint8_t);

  // Add minutes
  bool addMinutes(uint8_t);

  // Add hours
  bool addHours(uint8_t);

  // Add days
  bool addDays(uint8_t);

  // Returns a string with hour formatted as 'HH:MM:SS'
  std::string formatTime(bool = false) const;

  // Returns a string with date formatted as 'DD/MM/YYYY'
  std::string formatDate() const;

  friend class Clock;

  // TODO(matiascharrut) eliminate {} constructor.
};

// RTC clock driver
class Clock {
 private:
  I2C rtc;
  uint8_t address;

  static uint8_t bcd2dec(uint8_t);
  static uint8_t dec2bcd(uint8_t);

 public:
  Clock(PinName, PinName, uint8_t);

  // Sets time in RTC module with Time struct.
  bool set(const Time &);

  // Gets time from RTC module.
  Time get();

  // Syncs time with server and sets it in RTC module.
  bool sync(WiFi &, const std::string &);
};

#endif  // MODULES_CLOCK_CLOCK_H_
