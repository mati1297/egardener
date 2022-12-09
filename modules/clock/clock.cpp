// Copyright 2022 Matías Charrut
// This code is licensed under MIT license (see LICENSE for details)

#include <vector>
#include <string>
#include "mbed.h"
#include "clock.h"
#include "wifi.h"
#include "Json.h"

// CHEQUEAR QUE HAYA QUEDADO BIEN LO DEL DATE Y DAY. LO DE MODE TWELVE ETC. (CHEQUEAR BASICAMENTE QUE SE LEE BIEN LA HORA.)

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
  rtc.write(dec2bcd(time.hours) & 0x3F);

  rtc.write(1 & 0x07);
  rtc.write(dec2bcd(time.day) & 0x3F);
  rtc.write(dec2bcd(time.month) & 0x1F);
  rtc.write(dec2bcd(time.year));
  rtc.stop();
  return true;
}

Time Clock::get() {
  uint8_t seconds, minutes, hours, day, month, year;

  rtc.start();
  rtc.write(address | 0);
  rtc.write(0x00);

  rtc.start();

  rtc.write(address | 1);
  seconds = bcd2dec(rtc.read(1) & 0x7F);
  minutes = bcd2dec(rtc.read(1) & 0x7F);
  hours = bcd2dec(rtc.read(1) & 0x3F);

  rtc.read(1);
  day = bcd2dec(rtc.read(1) & 0x3F);
  month = bcd2dec(rtc.read(1) & 0x1F);
  year = bcd2dec(rtc.read(0));

  rtc.stop();

  return Time(seconds, minutes, hours, day, month, year);
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

  Time time(seconds, minutes, hours, date, month, year % 100);

  return set(time);
}

Time::Time(uint8_t seconds, uint8_t minutes, uint8_t hours, uint8_t day,
           uint8_t month, uint8_t year): seconds(seconds), minutes(minutes),
           hours(hours), day(day), month(month), year(year) {
  this->seconds = (this->seconds < 60) ? this->seconds : 0;
  this->minutes = (this->minutes < 60) ? this->minutes : 0;
  this->hours = (this->hours < 24) ? this->hours : 0;
  this->day = (this->day <= 31 && this->day > 0) ? this->day : 1;
  this->month = (this->month <= 12 && this->month > 0) ? this->month : 1;
  this->year = (this->year >= 0 && this->year <= 99) ? this->year : 0;
}

Time::Time(const Time& other) {
  seconds = other.seconds;
  minutes = other.minutes;
  hours = other.hours;
  day = other.day;
  month = other.month;
  year = other.year;
}

Time& Time::operator=(const Time& other) {
  seconds = other.seconds;
  minutes = other.minutes;
  hours = other.hours;
  day = other.day;
  month = other.month;
  year = other.year;
  return *this;
}

bool Time::addDays(uint8_t days) {
  if (days > MAX_DAYS_SUM)
    return false;

  uint8_t monthDays[] = MONTH_DAYS;

  uint8_t maxDaysMonth = monthDays[month - 1];
  if (month == 2 && year%4 == 0)
    maxDaysMonth = 29;

  bool carry;

  day += days;
  carry = day > maxDaysMonth;
  day -= maxDaysMonth * carry;

  month += carry;
  carry = month > 12;
  month -= carry * 12;

  year += carry;
  
  return true;
}

bool Time::addHours(uint8_t hours_) {
  if (hours_ > MAX_HOURS_SUM)
    return false;

  bool carry;
  hours += hours_;
  carry = hours >= 24;
  hours -= carry * 24;

  addDays(carry);

  return true;
}

bool Time::addMinutes(uint8_t minutes_) {
  if (minutes_ > MAX_MINUTES_SUM)
    return false;

  bool carry;
  minutes += minutes_;
  carry = minutes >= 60;
  minutes -= carry * 60;

  addHours(carry);

  return true;
}

bool Time::addSeconds(uint8_t seconds_) {
  if (seconds_ > MAX_SECONDS_SUM)
    return false;

  bool carry;
  seconds += seconds_;
  carry = seconds >= 60;
  seconds -= carry * 60;

  addMinutes(carry);

  return true;
}

bool Time::operator==(const Time& other) const {
  return seconds == other.seconds && minutes == other.minutes
  && hours == other.hours && day == other.day && month == other.month
  && year == other.year;
}

bool Time::operator<(const Time& other) const {
  if (*this == other)
    return false;
  
  if (year > other.year) return false;
  if (year < other.year) return true;
  if (month > other.month) return false;
  if (month < other.month) return true;
  if (day > other.day) return false;
  if (day < other.day) return true;
  if (hours > other.hours) return false;
  if (hours < other.hours) return true;
  if (minutes > other.minutes) return false;
  if (minutes < other.minutes) return true;
  if (seconds > other.seconds) return false;
  if (seconds < other.seconds) return true;

  return false;


  // TODO(matiascharrut) refactorizar.
  /*result = result && year <= other.year;
  result = result && month <= other.month;
  result = result && day <= other.day;
  result = result && hours <= other.hours;
  result = result && minutes <= other.minutes;
  result = result && seconds <= other.seconds;

  return result;*/
}

bool Time::operator<=(const Time& other) const {
  return *this < other || *this == other;
}

bool Time::operator>(const Time& other) const {
  return other < *this;
}

bool Time::operator>=(const Time& other) const {
  return *this > other || *this == other;
}

std::string Time::formatTime(bool twelve) const {
  if (twelve) {
    char buffer[BUFFER_SIZE_TIME_12 + 1];

    uint8_t mod_hours = hours;
    bool pm = mod_hours >= 12;

    if (pm)
      mod_hours -= 12;
    if (mod_hours == 0)
      mod_hours += 12;

    snprintf(buffer, sizeof(buffer), "%02u:%02u:%02u %s", mod_hours,
             minutes, seconds, pm ? "PM" : "AM");

    return std::string(buffer);
  } else {
    char buffer[BUFFER_SIZE_TIME_24 + 1];

    snprintf(buffer, sizeof(buffer), "%02u:%02u:%02u", hours,
             minutes, seconds);

    return std::string(buffer);
  }
}

std::string Time::formatDate() const {
  std::vector<char> buffer(BUFFER_SIZE_DATE + 1);

  snprintf(&buffer[0], BUFFER_SIZE_DATE + 1, "%02u/%02u/20%02u", day, month,
           year);

  return std::string(&buffer[0]);
}
