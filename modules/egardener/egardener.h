// Copyright 2022 Matías Charrut
// This code is licensed under MIT license (see LICENSE for details)

#ifndef MODULES_EGARDENER_EGARDENER_H_
#define MODULES_EGARDENER_EGARDENER_H_

#include <map>
#include <string>
#include <tuple>
#include <vector>
#include <utility>
#include "mbed.h"
#include "wifi.h"
#include "clock.h"
#include "memory.h"
#include "telegram_bot.h"
#include "credentials.h"
#include "trh_sensor.h"
#include "light_sensor.h"
#include "aux_functions.h"

#define I2C_PORT2_SDA_PIN PB_9
#define I2C_PORT2_SCL_PIN PB_8

#define WIFI_SERIAL_TX_PIN PD_5
#define WIFI_SERIAL_RX_PIN PD_6
#define WIFI_BAUDRATE 115200

#define LIGHT_SENSOR_PIN A1

#define ADDRESS_RTC 0x68
#define ADDRESS_EEPROM 0x50
#define ADDRESS_SENSOR_RH_TEMP 0x40

#define MAX_SSID_LENGTH 31  // para que entre en una pagina de la eeprom + un /0
#define MAX_PWD_LENGTH 31  // para que entre en una pagina de la eeprom.

#define WIFI_CONNECT_TRIES 3
#define TIMEZONE "America/Argentina/Buenos_Aires"
#define MAX_AMOUNT_TELEGRAM_MESSAGES 5

#define TELEGRAM_POLL_TIME 1000ms
#define TELEGRAM_POLL_TIME_WAITING 250ms
#define CLOCK_POLL_TIME 5s

#define TEMPERATURE_EMOJI "\xE2\x99\xA8"
#define HUMIDITY_EMOJI "\xF0\x9F\x92\xA7"
#define LIGHT_EMOJI "\xF0\x9F\x8C\x9E"

class eGardener {
 private:
  std::map<std::string, std::pair<uint16_t, uint8_t>> memoryDist;
  std::string wifi_ssid, wifi_pwd;
  WiFi wifi;
  Clock rtc;
  Memory eeprom;
  TRHSensor trhSensor;
  LightSensor lightSensor;
  TelegramBot bot;
  Ticker tickerCheckMessages, tickerCheckClock;
  bool checkMessages, checkClock, senseIntervalActivated;
  
  uint8_t senseInterval;
  char senseIntervalUnit;
  Time senseTargetTime;

  void setupMemoryDist();
  void setup();
  std::string getTelegramResponseForInteraction(TelegramBot &);
  void activateCheckMessages();
  void activateCheckClock();

  void sendWelcomeMessage(const std::string&);
  void sendTemperature(const std::string&);
  void sendHumidity(const std::string&);
  void sendLight(const std::string&);
  void sendSenseAll(const std::string&);
  void calibrateLightSensor(const std::string&);
  void setSenseInterval(const TelegramMessage&);
  void setSenseIntervalActivated(const std::string&, bool activated);
  void sendSenseIntervalStatus(const std::string&);
  void sendNextSenseTime(const std::string&);

  Time calculateTargetTime(uint8_t interval, char unit);

 public:
  eGardener();
  void execute();
};

#endif  // MODULES_EGARDENER_EGARDENER_H_
