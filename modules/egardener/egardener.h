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
#include "analog_sensor.h"
#include "aux_functions.h"
#include "control.h"
#include "periodic_action.h"
#include "activable_action.h"
#include "conditionable_action.h"
#include "user_register.h"

#define I2C_PORT2_SDA_PIN PB_9
#define I2C_PORT2_SCL_PIN PB_8

#define WIFI_SERIAL_TX_PIN PD_5
#define WIFI_SERIAL_RX_PIN PD_6
#define WIFI_BAUDRATE 115200

#define LIGHT_SENSOR_PIN A1
#define MOISTURE_SENSOR_PIN A0

#define LIGHT_PIN LED3
#define WATER_PIN LED2

#define ADDRESS_RTC 0x68
#define ADDRESS_EEPROM 0x50
#define ADDRESS_SENSOR_RH_TEMP 0x40

#define MAX_SSID_LENGTH 31  // para que entre en una pagina de la eeprom + un /0
#define MAX_PWD_LENGTH 31  // para que entre en una pagina de la eeprom.

#define WIFI_CONNECT_RETRIES 3
#define TIMEZONE "America/Argentina/Buenos_Aires"
#define MAX_AMOUNT_TELEGRAM_MESSAGES 5

#define TELEGRAM_POLL_TIME 1000ms
#define TELEGRAM_POLL_TIME_WAITING 250ms
#define CLOCK_POLL_TIME 5s
#define CONTROL_CONDITION_POLL_TIME 5s

#define TEMPERATURE_EMOJI "\xE2\x99\xA8"
#define MOISTURE_EMOJI "\xF0\x9F\x92\xA7"
#define LIGHT_EMOJI "\xF0\x9F\x8C\x9E"
#define HUMIDITY_EMOJI "\xE2\x98\x81"

#define CONTROL_HUMIDITY_CHAR 'h'
#define CONTROL_TEMPERATURE_CHAR 't'
#define CONTROL_LIGHT_CHAR 'l'
#define CONTROL_MOISTURE_CHAR 'm'

#define RESET_WIFI_PIN BUTTON1

#define TELEGRAM_RESPONSE_WAIT_TIMEOUT 240

// Principal class of the program, initializes objects, mantains flow,
// controls resources and communicates with user
class eGardener : public ActivableAction {
 private:
  std::map<std::string, std::pair<uint16_t, uint8_t>> memoryDist;
  std::string wifiSsid, wifiPwd;
  WiFi wifi;
  Clock rtc;
  Memory eeprom;
  TRHSensor trhSensor;
  AnalogSensor lightSensor, moistureSensor;
  TelegramBot bot;
  Control controlLight, controlWater;
  Ticker tickerCheckMessages, tickerCheckClock, tickerCheckControlCondition;
  InterruptIn interruptResetWiFi;
  bool checkMessages, checkClock, checkControlCondition, checkResetWiFi, checkWiFiConnected;

  bool controlWaterManually, controlLightManually;
  
  PeriodicAction periodicSense, periodicWater, periodicLight;
  ConditionableAction conditionableWater, conditionableLight;

  UserRegister userRegister;

  void setupMemoryDist();
  void setup();
  std::string getTelegramResponseForInteraction();
  void activateCheckMessages();
  void activateCheckClock();
  void activateCheckControlCondition();
  void activateResetWiFi();

  void connectToWiFi();

  void sendWelcomeMessage(const std::string&);
  void addUser(const std::string&, const std::string&);
  void setPwd(const std::string&, const std::string&);
  void removeUser(const std::string&);
  void sendTemperature(const std::string&);
  void sendHumidity(const std::string&);
  void sendLight(const std::string&);
  void sendMoisture(const std::string&);
  void sendSenseAll(const std::string&);
  void calibrateLightSensor(const std::string&);
  void calibrateMoistureSensor(const std::string&);
  void setSenseInterval(const std::string&, const std::string&);
  void setSenseIntervalActivated(const std::string&, bool activated);
  void sendSenseIntervalStatus(const std::string&);
  void sendNextSenseTime(const std::string&);
  void setControlConditions(const std::string&, const std::string&);
  void setControlInterval(const std::string&, const std::string&);
  void setControlStatus(const std::string&, const std::string&, bool);
  void setControlIntervalStatus(const std::string&, const std::string&, bool);
  void setControlConditionsStatus(const std::string&, const std::string&, bool);
  void sendNextControlTime(const std::string&, const std::string&);
  void sendControlIntervalStatus(const std::string&, const std::string&);
  void sendControlStatus(const std::string&, const std::string&);
  void sendControlConditionStatus(const std::string&, const std::string&);

  void resetWiFi();
  bool checkAndSaveNewWiFi();

  void activate();
  void deactivate();

 public:
  eGardener();

  // Executes eGardener program.
  void execute();
};

#endif  // MODULES_EGARDENER_EGARDENER_H_
