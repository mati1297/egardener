// Copyright 2022 Matías Charrut
// This code is licensed under MIT license (see LICENSE for details)

#include <vector>
#include <string>
#include "mbed.h"
#include "clock.h"
#include "modules/wifi/wifi.h"
#include "modules/Json/Json.h"

Clock::Clock(PinName SDA, PinName SCL, uint8_t address): rtc(SDA, SCL),
                                                         address(address << 1) {
  rtc.start();
  rtc.write(address | 0);
  rtc.write(0x07u);
  rtc.write(0x00u);
  rtc.stop();
}

uint8_t Clock::bcd2dec(uint8_t bcd) {
  return (bcd & 0x0F) + (((bcd & 0xF0) >> 4) * 10);
}

uint8_t Clock::dec2bcd(uint8_t dec) {
  return ((dec % 10) & 0x0F) | ((dec / 10) << 4 & 0xF0);
}

bool Clock::set(const Time& time) {
  rtc.start();
  rtc.write(address | 0);
  rtc.write(0x00u);
  rtc.write(dec2bcd(time.seconds) & 0x7F);
  rtc.write(dec2bcd(time.minutes) & 0x7F);
  if (time.mode_twelve)
    rtc.write(0x40 | (time.pm << 5 & 0x20) | (dec2bcd(time.hours) & 0x1F));
  else
    rtc.write(dec2bcd(time.hours) & 0x3F);

  rtc.write(time.day & 0x07);
  rtc.write(dec2bcd(time.date) & 0x3F);
  rtc.write(dec2bcd(time.month) & 0x1F);
  rtc.write(dec2bcd(time.year));
  rtc.stop();
  return true;
}

Time Clock::get() {
  uint8_t seconds, minutes, hours, day, date, month, year;
  bool pm, mode_twelve;
  uint8_t hours_read;

  rtc.start();
  rtc.write(address | 0);
  rtc.write(0x00);

  rtc.start();

  rtc.write(address | 1);
  seconds = bcd2dec(rtc.read(1) & 0x7F);
  minutes = bcd2dec(rtc.read(1) & 0x7F);
  hours_read = rtc.read(1);
  if (hours_read & 0x40) {
    hours = bcd2dec(hours_read & 0x1F);
    pm = hours_read & 0x20;
    mode_twelve = true;
  } else {
    hours = bcd2dec(hours_read & 0x3F);
    mode_twelve = false;
  }

  day = rtc.read(1) & 0x07;
  date = bcd2dec(rtc.read(1) & 0x3F);
  month = bcd2dec(rtc.read(1) & 0x1F);
  year = bcd2dec(rtc.read(0));

  rtc.stop();

  return Time(seconds, minutes, hours, pm, mode_twelve, day, date, month,
              year);
}

// agregar twelve.
bool Clock::sync(WiFi& wifi, const std::string& timezone) {
  std::string url = "https://www.timeapi.io/api/Time/current/zone?timeZone="
                    + timezone;

  std::string response = wifi.get(url);

  Json json(response.c_str(), response.length(), 30);

  if (!json.isValidJson())
    return false;

  int seconds, minutes, hours, date, month, year;

  json.tokenIntegerValue(json.findChildIndexOf(json.findKeyIndex("seconds")),
                         seconds);
  json.tokenIntegerValue(json.findChildIndexOf(json.findKeyIndex("minute")),
                         minutes);
  json.tokenIntegerValue(json.findChildIndexOf(json.findKeyIndex("hour")),
                         hours);
  json.tokenIntegerValue(json.findChildIndexOf(json.findKeyIndex("day")),
                         date);
  json.tokenIntegerValue(json.findChildIndexOf(json.findKeyIndex("month")),
                         month);
  json.tokenIntegerValue(json.findChildIndexOf(json.findKeyIndex("year")),
                         year);

  std::string dayOfWeek = json.tokenString(json.findChildIndexOf(
                                           json.findKeyIndex("dayOfWeek")));

  Time time(seconds, minutes, hours, false, false,
            Time::dayWordToDayNumber(dayOfWeek), date, month, year % 100);

  return set(time);
}

Time::Time(uint8_t seconds, uint8_t minutes, uint8_t hours, bool pm,
           bool mode_twelve, uint8_t day, uint8_t date, uint8_t month,
           uint8_t year): seconds(seconds), minutes(minutes), hours(hours),
                          day(day), date(date), month(month), year(year),
                          pm(pm), mode_twelve(mode_twelve) {
  seconds = (seconds < 60) ? seconds : 0;
  minutes = (minutes < 60) ? minutes : 0;
  if (mode_twelve)
    hours = (hours <= 12 && hours >= 1) ? hours : 12;
  else
    hours = (hours < 24) ? hours : 0;

  day = (day <= 7 && day > 0) ? day : 1;
  date = (date <= 31 && date > 0) ? date : 1;  // validar por mes?
  month = (month <= 12 && month > 0) ? month : 1;
  year = (year >= 0 && year <= 99) ? year : 0;
}

std::string Time::formatTime() const {
  if (mode_twelve) {
    std::vector<char> buffer(BUFFER_SIZE_TIME_12 + 1);

    snprintf(&buffer[0], BUFFER_SIZE_TIME_12 + 1, "%02u:%02u:%02u %s", hours,
             minutes, seconds, pm ? "PM" : "AM");

    return std::string(&buffer[0]);
  } else {
    std::vector<char> buffer(BUFFER_SIZE_TIME_24 + 1);

    snprintf(&buffer[0], BUFFER_SIZE_TIME_24 + 1, "%02u:%02u:%02u", hours,
             minutes, seconds);

    return std::string(&buffer[0]);
  }
}

std::string Time::formatDate() const {
  std::vector<char> buffer(BUFFER_SIZE_DATE + 1);

  snprintf(&buffer[0], BUFFER_SIZE_DATE + 1, "%02u/%02u/20%02u", date, month,
           year);

  return std::string(&buffer[0]);
}

std::string Time::formatDayWeek() const {
  return dayNumberToDayWord(day);
}

std::string Time::dayNumberToDayWord(uint8_t dayNumber) {
  std::string days[] = DAYS_OF_WEEK;

  if (dayNumber == 0 || dayNumber > 7)
    return days[0];

  return days[dayNumber - 1];
}

uint8_t Time::dayWordToDayNumber(const std::string &dayWord) {
  std::string days[] = DAYS_OF_WEEK;

  for (uint8_t i = 0; i < 7; i++) {
    if (dayWord == days[i])
      return i + 1;
  }

  return 1;
}