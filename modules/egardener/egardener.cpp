// Copyright 2022 Matías Charrut
// This code is licensed under MIT license (see LICENSE for details)

#include <map>
#include <tuple>
#include <string>
#include <vector>
#include <algorithm>
#include <utility>
#include "mbed.h"
#include "egardener.h"
#include "wifi.h"
#include "clock.h"
#include "memory.h"
#include "telegram_bot.h"
#include "credentials.h"
#include "trh_sensor.h"
#include "analog_sensor.h"
#include "aux_functions.h"
#include "control.h"
#include "conditionable_action.h"
#include "activable_action.h"
#include "user_register.h"

// hacer que guarde un user y listo.
// o hacer con contraseña?

// TODO(matiascharrut) guardar.
eGardener::eGardener(): memoryDist(),
            wifi(WIFI_SERIAL_TX_PIN, WIFI_SERIAL_RX_PIN,
               WIFI_BAUDRATE),
            wifiSsid(), wifiPwd(),
            rtc(I2C_PORT2_SDA_PIN, I2C_PORT2_SCL_PIN,
              ADDRESS_RTC),
            eeprom(I2C_PORT2_SDA_PIN, I2C_PORT2_SCL_PIN,
                 ADDRESS_EEPROM),
            trhSensor(I2C_PORT2_SDA_PIN, I2C_PORT2_SCL_PIN,
                  ADDRESS_SENSOR_RH_TEMP),
            lightSensor(LIGHT_SENSOR_PIN), moistureSensor(MOISTURE_SENSOR_PIN), bot(wifi, TELEGRAM_BOT_TOKEN), controlLight(LIGHT_PIN), controlWater(WATER_PIN),
            tickerCheckMessages(), tickerCheckClock(), tickerCheckControlCondition(), interruptResetWiFi(RESET_WIFI_PIN),
            checkMessages(false), checkClock(false), checkControlCondition(false), checkResetWiFi(false), checkWiFiConnected(false),
            controlWaterManually(false), controlLightManually(false),
            periodicSense(*this), periodicWater(controlWater, false, false), periodicLight(controlLight, false, false),
            conditionableWater(controlWater, std::vector<char>{CONTROL_HUMIDITY_CHAR, CONTROL_LIGHT_CHAR, CONTROL_TEMPERATURE_CHAR, CONTROL_MOISTURE_CHAR}),
            conditionableLight(controlLight, std::vector<char>{CONTROL_HUMIDITY_CHAR, CONTROL_LIGHT_CHAR, CONTROL_TEMPERATURE_CHAR, CONTROL_MOISTURE_CHAR}),
            userRegister() {
  setup();
}

void eGardener::execute() {
  // validar de quien viene y guardar en memoria.
  while (true) {
    if (checkMessages) {
      std::vector<TelegramMessage> messages = bot.getMessages(
                                              MAX_AMOUNT_TELEGRAM_MESSAGES);
      for (auto message : messages) {
        size_t firstSpacePos = message.text.find_first_of(' ');
        std::string cmd = message.text.substr(0, firstSpacePos);
        std::string body = message.text.substr(firstSpacePos + 1);
        if (message.text == "/start") {
          sendWelcomeMessage(message.from_id);
          continue;
        }
        if (cmd == "/addme") {
          addUser(message.from_id, body);
          continue;
        }
        if (!userRegister.isUserRegistered(message.from_id)) {
          bot.sendMessage(message.from_id, "You need to be registered. Use /addme followed by password to register.");
          continue;
        }
        if (firstSpacePos != std::string::npos && cmd == "/setpwd")
          setPwd(message.from_id, body);
        else if (message.text == "/removeme")
          removeUser(message.from_id);
        else if (message.text == "/temperature")
          sendTemperature(message.from_id);
        else if (message.text == "/humidity")
          sendHumidity(message.from_id);
        else if (message.text == "/light")
          sendLight(message.from_id);
        else if (message.text == "/moisture")
          sendMoisture(message.from_id);
        else if (message.text == "/calibratelightsensor")
          calibrateLightSensor(message.from_id);
        else if (message.text == "/calibratemoisturesensor")
          calibrateMoistureSensor(message.from_id);
        else if(message.text == "/senseall")
          sendSenseAll(message.from_id);
        else if (message.text == "/activatesenseinterval")
          setSenseIntervalActivated(message.from_id, true); // si se activa queda desde el ultimo.
        else if (message.text == "/deactivatesenseinterval")
          setSenseIntervalActivated(message.from_id, false);
        else if (message.text == "/senseintervalstatus")
          sendSenseIntervalStatus(message.from_id);
        else if (firstSpacePos != std::string::npos && cmd == "/setsenseinterval")
          setSenseInterval(message.from_id, message.text.substr(firstSpacePos+1));
        else if (message.text == "/nextsensetime")
          sendNextSenseTime(message.from_id);
        else if (firstSpacePos != std::string::npos && cmd == "/activatecontrolcondition")
          setControlConditionsStatus(message.from_id, body, true);
        else if (firstSpacePos != std::string::npos && cmd == "/deactivatecontrolcondition")
          setControlConditionsStatus(message.from_id, body, false);
        else if (firstSpacePos != std::string::npos && cmd == "/setcontrolcondition")
          setControlConditions(message.from_id, body);
        else if (firstSpacePos != std::string::npos && cmd == "/setcontrolinterval")
          setControlInterval(message.from_id, body);
        else if (firstSpacePos != std::string::npos && cmd == "/activatecontrolinterval")
          setControlIntervalStatus(message.from_id, body, true);
        else if (firstSpacePos != std::string::npos && cmd == "/deactivatecontrolinterval")
          setControlIntervalStatus(message.from_id, body, false);
        else if (firstSpacePos != std::string::npos && cmd == "/controlintervalstatus")
          sendControlIntervalStatus(message.from_id, body);
        else if (firstSpacePos != std::string::npos && cmd == "/nextcontroltime")
          sendNextControlTime(message.from_id, body);
        else if (firstSpacePos != std::string::npos && cmd == "/activatecontrol")
          setControlStatus(message.from_id, body, true);
        else if (firstSpacePos != std::string::npos && cmd == "/deactivatecontrol")
          setControlStatus(message.from_id, body, false);
        else if (firstSpacePos != std::string::npos && cmd == "/controlstatus")
          sendControlStatus(message.from_id, body);
        else if (firstSpacePos != std::string::npos && cmd == "/controlconditionstatus")
          sendControlConditionStatus(message.from_id, body);
        else
          bot.sendMessage(message.from_id, "Command unknown");
      }
      checkMessages = false;
    }
    if (checkClock) {
      Time time = rtc.get(); 
      periodicSense.execute(time);
      periodicLight.execute(time);
      periodicWater.execute(time);
    }
    if (checkControlCondition) {
      std::map<char, uint8_t> values{{CONTROL_HUMIDITY_CHAR, trhSensor.senseHumidity()},
                                     {CONTROL_TEMPERATURE_CHAR, trhSensor.senseTemperature()},
                                     {CONTROL_LIGHT_CHAR, lightSensor.sense() * 100},
                                     {CONTROL_MOISTURE_CHAR, moistureSensor.sense() * 100}};
      conditionableWater.execute(values);
      conditionableLight.execute(values);
      checkControlCondition = false;
    }
    if (checkResetWiFi) {
      resetWiFi();
      ThisThread::sleep_for(1s);
      checkWiFiConnected = true;
      checkResetWiFi = false;
    }
    if (checkWiFiConnected) {
      if (checkAndSaveNewWiFi())
        checkWiFiConnected = false;
    }
    // Por ahora, despues con timers y booleans.
  }
}

void eGardener::activateCheckMessages() {
  checkMessages = true;
}

void eGardener::activateCheckClock() {
  checkClock = true;
}

void eGardener::activateCheckControlCondition() {
  checkControlCondition = true;
}

void eGardener::activateResetWiFi() {
  checkResetWiFi = true;
}

void eGardener::sendWelcomeMessage(const std::string& user_id) {
  if (userRegister.isUserRegistered(user_id)) 
    bot.sendMessage(user_id, "Welcome to eGardener!");
  else
    bot.sendMessage(user_id, "Welcome to eGardener, you are not registered. Register with /addme followed by the password");

  //HELP
}

void eGardener::setPwd(const std::string& user_id, const std::string& body) {
  size_t firstSpacePos = body.find_first_of(' ');

  std::string newPwd = body.substr(0, firstSpacePos);
  std::string oldPwd = body.substr(firstSpacePos+1);

  if (newPwd.find_first_of(' ') != std::string::npos) {
    bot.sendMessage(user_id, "Password must not have spaces");
    return;
  }
  else if (userRegister.setPwd(oldPwd, newPwd))
    bot.sendMessage(user_id, "Password set succesfully");
  else {
    bot.sendMessage(user_id, "Old password is incorrect or new password is longer than " + to_string(MAX_PWD_USER_LENGTH));
    return;
  }

  auto address = memoryDist.find("ur")->second;
  eeprom.write(address.first, true);
  eeprom.write(address.first + sizeof(bool), newPwd);

  eeprom.read(address.first + sizeof(bool), newPwd);
}

void eGardener::addUser(const std::string& user_id, const std::string& body) {
  uint8_t result = userRegister.addUser(user_id, body);

  if (result == 1) {
    bot.sendMessage(user_id, "You were added already");
  }
  if (result == 2) {
    bot.sendMessage(user_id, "Wrong password");
    return;
  }
  else if (result == 3) {
    bot.sendMessage(user_id, "Cannot save more users");
    return;
  }

  bot.sendMessage(user_id, "You were succesfully added");

  // save.
  auto users = userRegister.getUsers();

  auto address = memoryDist.find("ur")->second;
  eeprom.write(address.first, true);
  eeprom.write(address.first + sizeof(bool) + MAX_PWD_USER_LENGTH + 1, users.size());
  for (int i = 0; i < users.size(); i++) {
    eeprom.write(address.first + sizeof(bool) + MAX_PWD_USER_LENGTH + 1 + sizeof(uint8_t) + sizeof(uint32_t) * i, users[i]);
  }
}

void eGardener::removeUser(const std::string& user_id) {
  if (!userRegister.removeUser(user_id))
    return;

  bot.sendMessage(user_id, "You were succesfully removed");
  
  auto users = userRegister.getUsers();

  auto address = memoryDist.find("ur")->second;
  eeprom.write(address.first, true);
  eeprom.write(address.first + sizeof(bool) + MAX_PWD_USER_LENGTH + 1, users.size());
  for (int i = 0; i < users.size(); i++) {
    eeprom.write(address.first + sizeof(bool) + MAX_PWD_USER_LENGTH + 1 + sizeof(uint8_t) + sizeof(uint32_t) * i, users[i]);
  }
}

void eGardener::sendTemperature(const std::string& user_id) {
  Time time = rtc.get();
  bot.sendMessage(user_id, "[" + time.formatTime() + " " + time.formatDate() + "] " +
                  TEMPERATURE_EMOJI + (" " + 
                  floatToString(trhSensor.senseTemperature(), 2)) + 
                  "°C");
}

void eGardener::sendHumidity(const std::string& user_id) {
  Time time = rtc.get();
  bot.sendMessage(user_id, "[" + time.formatTime() + " " + time.formatDate() + "] " +
                  HUMIDITY_EMOJI + (" " + 
                  floatToString(trhSensor.senseHumidity(), 2)) + 
                  "%");
}

void eGardener::sendMoisture(const std::string& user_id) {
  Time time = rtc.get();
  float value = moistureSensor.sense();
  if (value < 0)
    bot.sendMessage(user_id,
                    R"(You need to calibrate moisture sensor with )" +
                    std::string(R"(/calibratemoisturesensor)"));
  else
    bot.sendMessage(user_id, "[" + time.formatTime() + " " + time.formatDate() + "] " +
                    MOISTURE_EMOJI + (" " + 
                    floatToString(value * 100, 2)) + "%");
}

void eGardener::sendLight(const std::string& user_id) {
  Time time = rtc.get();
  float value = lightSensor.sense();
  if (value < 0)
    bot.sendMessage(user_id,
                    R"(You need to calibrate light sensor with )" +
                    std::string(R"(/calibratelightsensor)"));
  else
    bot.sendMessage(user_id, "[" + time.formatTime() + " " + time.formatDate() + "] " +
                    LIGHT_EMOJI + (" " + 
                    floatToString(value * 100, 2)) + "%");
}

void eGardener::sendSenseAll(const std::string& user_id) {
  float valueLight = lightSensor.sense();
  float valueMoisture = moistureSensor.sense();
  if (valueLight < 0) {
    bot.sendMessage(user_id,
                    R"(You need to calibrate light sensor with )" +
                    std::string(R"(/calibratelightsensor)"));
  } else if (valueMoisture < 0) {
    bot.sendMessage(user_id,
                R"(You need to calibrate moisture sensor with )" +
                    std::string(R"(/calibratemoisturesensor)"));
  } else {
    Time time = rtc.get();
    std::string messageTime = "[" + time.formatTime() + " " + time.formatDate() + "]";
    std::string temp = TEMPERATURE_EMOJI + (" " +
                  floatToString(trhSensor.senseTemperature(), 2)) + 
                  "°C";
    std::string moisture = MOISTURE_EMOJI + (" " +
                  floatToString(valueMoisture * 100, 2)) + "%";
    std::string humidity = HUMIDITY_EMOJI + (" " + 
                  floatToString(trhSensor.senseHumidity(), 2)) + 
                  "% ";
    std::string light = LIGHT_EMOJI + (" " +
                  floatToString(valueLight * 100, 2)) + "%";
    bot.sendMessage(user_id, messageTime + "\n" + temp + "\n" + moisture + "\n" + humidity + "\n" + light);
  }
}

void eGardener::calibrateLightSensor(const std::string& user_id) {
  float min, max;

  bot.sendMessage(user_id, R"(Put direct light on the sensor )" + 
                           std::string(R"(with your phone flash and type ok)")
                           + R"( (any other text to cancel).)");
  if (getTelegramResponseForInteraction() != "ok") {
    bot.sendMessage(user_id, "Operation cancelled");
    return;
  }
  max = lightSensor.sense(true);

  bot.sendMessage(user_id, std::string(R"(Now put your finger on the sensor)")
                           + R"(and type ok (any other text to cancel).)");
  if (getTelegramResponseForInteraction() != "ok") {
              bot.sendMessage(user_id, "Operation cancelled");
    return;
  }
  min = lightSensor.sense(true);

  if (lightSensor.setMaxAndMin(max, min)) {
    bot.sendMessage(user_id, R"(Light sensor calibrated succesfully)");
    auto address = memoryDist.find("ls")->second;
    eeprom.write(address.first, true);
    eeprom.write(address.first + sizeof(bool), max);
    eeprom.write(address.first + sizeof(bool) + sizeof(float), min);
  } else {
    bot.sendMessage(user_id, "An error ocurred, try again");
  }
}

void eGardener::calibrateMoistureSensor(const std::string& user_id) {
  float min, max;

  bot.sendMessage(user_id, "Keep your moisture sensor in the air and type ok (any other text to cancel)");
  if (getTelegramResponseForInteraction() != "ok") {
    bot.sendMessage(user_id, "Operation cancelled");
    return;
  }
  min = moistureSensor.sense(true);

  bot.sendMessage(user_id, "Put your moisture sensor in a glass with water (IMPORTANT: leave half a centimetre below the line), wait at least 20 seconds and type ok (any other text to cancel)");
  if (getTelegramResponseForInteraction() != "ok") {
              bot.sendMessage(user_id, "Operation cancelled");
    return;
  }
  max = moistureSensor.sense(true);

  if (lightSensor.setMaxAndMin(max, min)) {
    bot.sendMessage(user_id, R"(Moisture sensor calibrated succesfully)");
    auto address = memoryDist.find("ms")->second;
    eeprom.write(address.first, true);
    eeprom.write(address.first + sizeof(bool), max);
    eeprom.write(address.first + sizeof(bool) + sizeof(float), min);
  } else {
    bot.sendMessage(user_id, "An error ocurred, try again");
  }
}

void eGardener::setSenseIntervalActivated(const std::string& user_id, bool activated) {
  periodicSense.setActivatedStatus(activated);

  auto address = memoryDist.find("st")->second;
  eeprom.write(address.first, true);
  eeprom.write(address.first + sizeof(bool), activated);
  
  std::string state = (activated ? "activated" : "deactivated");
  

  bot.sendMessage(user_id, "Sense interval has been " + state);
}

void eGardener::sendSenseIntervalStatus(const std::string& user_id) {
  std::string state = periodicSense.isActivated() ? "activated" : "deactivated";
  bot.sendMessage(user_id, "Sense interval is " + state +
                  "\nSense interval is " + to_string(periodicSense.getInterval()) +
                  periodicSense.getIntervalUnit());
}

void eGardener::sendNextSenseTime(const std::string& user_id) {
  bot.sendMessage(user_id, "Next sense at " + periodicSense.getNextTime().formatTime()
                          + " " + periodicSense.getNextTime().formatDate());
}

void eGardener::sendNextControlTime(const std::string& user_id, const std::string& body) {
  PeriodicAction * control;

  if (body[0] == 'w')
    control = &periodicWater;
  else if (body[0] == 'l')
    control = &periodicLight;
  else {
    bot.sendMessage(user_id, "Control selected unknown");
    return;
  }

  bot.sendMessage(user_id, "Next sense at " + control->getNextTime().formatTime()
                          + " " + control->getNextTime().formatDate());
}

void eGardener::sendControlIntervalStatus(const std::string& user_id, const std::string& body) {
  PeriodicAction * control;

  if (body[0] == 'w')
    control = &periodicWater;
  else if (body[0] == 'l')
    control = &periodicLight;
  else {
    bot.sendMessage(user_id, "Control selected unknown");
    return;
  }

  std::string state = control->isActivated() ? "activated" : "deactivated";
  bot.sendMessage(user_id, std::string("Control interval ") + body[0] + " is " + state +
                  "\nControl interval is " + to_string(control->getInterval()) +
                  control->getIntervalUnit() + " with duration " + to_string(control->getDuration()) + control->getDurationUnit());
}

void eGardener::sendControlStatus(const std::string& user_id, const std::string& body) {
  Control * control;
  bool manually;

  if (body[0] == 'w') {
    control = &controlWater;
    manually = controlWaterManually;
  }
  else if (body[0] == 'l') {
    control = &controlLight;
    manually = controlLightManually;
  }
  else {
    bot.sendMessage(user_id, "Control selected unknown");
    return;
  }

  std::string state = control->isActivated() ? "activated" : "deactivated";
  bot.sendMessage(user_id, std::string("Control ") + body[0] + " is " + state
                  + ((manually) ? " (manually)" : ""));
}

void eGardener::sendControlConditionStatus(const std::string& user_id, const std::string& body) {
  ConditionableAction * control;

  if (body[0] == 'w') {
    control = &conditionableWater;
  }
  else if (body[0] == 'l') {
    control = &conditionableLight;

  }
  else {
    bot.sendMessage(user_id, "Control selected unknown");
    return;
  }

  std::string conditionsStr;
  auto conditions = control->getConditions();

  for (auto it = conditions.begin(); it != conditions.end(); it++) {
    conditionsStr += it->first + std::string(" ") + ((it->second.first == ConditionableAction::Symbol::GREAT) ? GREAT_CHAR : LESS_CHAR);
    conditionsStr += " " + to_string(it->second.second) + "\n";
  }

  std::string state = control->isActivated() ? "activated" : "deactivated";
  bot.sendMessage(user_id, std::string("Control condition ") + body[0] + " is " + state +
                  "\nConditions are\n" + conditionsStr);
}

void eGardener::setSenseInterval(const std::string& user_id, const std::string& body) {
  // Crear macros
  if (periodicSense.setInterval(body, rtc.get())) {
    auto address = memoryDist.find("st")->second;
    eeprom.write(address.first, true);
    eeprom.write(address.first + sizeof(bool) * 2, periodicSense.getInterval());
    eeprom.write(address.first + sizeof(bool) * 2 + sizeof(uint8_t), periodicSense.getIntervalUnit());

    bot.sendMessage(user_id, "Sense interval set " + std::to_string(periodicSense.getInterval())
                                     + periodicSense.getIntervalUnit());
  } else {
    bot.sendMessage(user_id, "El intervalo o la unidad no son correctos");
  }
}

void eGardener::setup() {
  printf("Initializing...\n");
  printf("Loading data from EEPROM...\n");
  setupMemoryDist();
  bool there;
  // leo wifi.
  auto address = memoryDist.find("wifi")->second;
  // validar que lo encuentre?
  eeprom.read(address.first, there);
  if (there) {
    eeprom.read(address.first + sizeof(bool), wifiSsid);
    eeprom.read(address.first + sizeof(bool) + MAX_SSID_LENGTH + 1, wifiPwd);
  }

  address = memoryDist.find("ls")->second;
  eeprom.read(address.first, there);
  if (there) {
    float ls_max, ls_min;
    eeprom.read(address.first + sizeof(bool), ls_max);
    eeprom.read(address.first + sizeof(bool) + sizeof(float), ls_min);
    lightSensor.setMaxAndMin(ls_max, ls_min);
  }
  
  address = memoryDist.find("st")->second;
  
  eeprom.read(address.first, there);
  if (there) {
    uint8_t interval;
    char intervalUnit;
    bool activated;
    eeprom.read(address.first + sizeof(bool), activated);
    eeprom.read(address.first + sizeof(bool) * 2, interval);
    eeprom.read(address.first + sizeof(bool) * 2 + sizeof(uint8_t), intervalUnit);
    periodicSense.setActivatedStatus(activated);
    periodicSense.setInterval(interval, intervalUnit, rtc.get());
  }
  
  address = memoryDist.find("ctw")->second;

  eeprom.read(address.first, there);
  if (there) {
    uint8_t interval, duration;
    char intervalUnit, durationUnit;
    bool activated;
    eeprom.read(address.first + sizeof(bool), activated);
    eeprom.read(address.first + sizeof(bool) * 2, interval);
    eeprom.read(address.first + sizeof(bool) * 2 + sizeof(uint8_t), intervalUnit);
    eeprom.read(address.first + sizeof(bool) * 2 + sizeof(uint8_t) + sizeof(char), duration);
    eeprom.read(address.first + sizeof(bool) * 2 + sizeof(uint8_t) * 2 + sizeof(char), durationUnit);
    periodicWater.setActivatedStatus(activated);
    periodicWater.setInterval(interval, intervalUnit, rtc.get());
    periodicWater.setDuration(duration, durationUnit);
  }

  address = memoryDist.find("ctl")->second;

  eeprom.read(address.first, there);
  if (there) {
    uint8_t interval, duration;
    char intervalUnit, durationUnit;
    bool activated;
    eeprom.read(address.first + sizeof(bool), activated);
    eeprom.read(address.first + sizeof(bool) * 2, interval);
    eeprom.read(address.first + sizeof(bool) * 2 + sizeof(uint8_t), intervalUnit);
    eeprom.read(address.first + sizeof(bool) * 2 + sizeof(uint8_t) + sizeof(char), duration);
    eeprom.read(address.first + sizeof(bool) * 2 + sizeof(uint8_t) * 2 + sizeof(char), durationUnit);
    periodicLight.setActivatedStatus(activated);
    periodicLight.setInterval(interval, intervalUnit, rtc.get());
    periodicLight.setDuration(duration, durationUnit);
  }

  address = memoryDist.find("ccw")->second;

  eeprom.read(address.first, there);
  if (there) {
    std::map<char, ConditionableAction::ConditionPair> conditions;
    bool activated;
    uint8_t amount;
    uint16_t pos_accum = 0;
    eeprom.read(address.first + sizeof(bool), activated);
    eeprom.read(address.first + sizeof(bool) * 2, amount);
    for (uint8_t i = 0; i < amount; i++) {
      char variable;
      uint8_t symbol, value;
      eeprom.read(address.first + sizeof(bool) * 2 + sizeof(uint8_t) + pos_accum, variable);
      eeprom.read(address.first + sizeof(bool) * 2 + sizeof(uint8_t) + sizeof(char) + pos_accum, symbol);
      eeprom.read(address.first + sizeof(bool) * 2 + sizeof(uint8_t) + sizeof(char) + sizeof(uint8_t) + pos_accum, value);
      pos_accum += sizeof(char) + sizeof(uint8_t) * 2;
      conditions.insert(std::make_pair(variable, ConditionableAction::ConditionPair((ConditionableAction::Symbol) symbol, value)));
    }
    conditionableWater.setConditions(conditions);
  }

  address = memoryDist.find("ccl")->second;
  eeprom.read(address.first, there);
  if (there) {
    std::map<char, ConditionableAction::ConditionPair> conditions;
    bool activated;
    uint8_t amount;
    uint16_t pos_accum = 0;
    eeprom.read(address.first + sizeof(bool), activated);
    eeprom.read(address.first + sizeof(bool) * 2, amount);
    for (uint8_t i = 0; i < amount; i++) {
      char variable;
      uint8_t symbol, value;
      eeprom.read(address.first + sizeof(bool) * 2 + sizeof(uint8_t) + pos_accum, variable);
      eeprom.read(address.first + sizeof(bool) * 2 + sizeof(uint8_t) + sizeof(char) + pos_accum, symbol);
      eeprom.read(address.first + sizeof(bool) * 2 + sizeof(uint8_t) + sizeof(char) + sizeof(uint8_t) + pos_accum, value);
      pos_accum += sizeof(char) + sizeof(uint8_t) * 2;
      conditions.insert(std::make_pair(variable, ConditionableAction::ConditionPair((ConditionableAction::Symbol) symbol, value)));
    }
    conditionableLight.setConditions(conditions);
  }

  address = memoryDist.find("ms")->second;
  eeprom.read(address.first, there);
  if (there) {
    float ls_max, ls_min;
    eeprom.read(address.first + sizeof(bool), ls_max);
    eeprom.read(address.first + sizeof(bool) + sizeof(float), ls_min);
    moistureSensor.setMaxAndMin(ls_max, ls_min);
  }

  address = memoryDist.find("ur")->second;
  eeprom.read(address.first, there);

  if (there) {
    std::string pwd;
    uint8_t usersSize;
    eeprom.read(address.first + sizeof(bool), pwd);
    printf("Password: %s\n", pwd.c_str());
    userRegister.setPwd("", pwd);
    eeprom.read(address.first + sizeof(bool) + MAX_PWD_USER_LENGTH + 1, usersSize);
    for (int i = 0; i < usersSize; i++) {
      uint32_t user;
      eeprom.read(address.first + sizeof(bool) + MAX_PWD_USER_LENGTH + 1 + sizeof(uint8_t) + sizeof(uint32_t) * i, user);
      userRegister.addUser(user, pwd);
    }
  }

  printf("Connecting to Wi-Fi...\n");

  connectToWiFi();

  printf("Setting up tickers and interruptions...\n");
  // setea tickers
  tickerCheckMessages.attach(callback(this, &eGardener::activateCheckMessages),
                TELEGRAM_POLL_TIME);
  tickerCheckClock.attach(callback(this, &eGardener::activateCheckClock),
                CLOCK_POLL_TIME);
  tickerCheckControlCondition.attach(callback(this, &eGardener::activateCheckControlCondition),
                CONTROL_CONDITION_POLL_TIME);
  interruptResetWiFi.rise(callback(this, &eGardener::activateResetWiFi));
  

  printf("Ready!\n");
}

// devuelve todo en minusculas.
// Se le podri agregar para que tome del mismo usuario si o si.
std::string eGardener::getTelegramResponseForInteraction() {
  std::vector<TelegramMessage> messages;
  uint8_t counter = 0;
  while (messages.empty() && counter < TELEGRAM_RESPONSE_WAIT_TIMEOUT) {
    messages = bot.getMessages(1);
    ThisThread::sleep_for(TELEGRAM_POLL_TIME_WAITING);
    counter++;
  }
  std::string response;
  if (counter == TELEGRAM_RESPONSE_WAIT_TIMEOUT)
    response = "";
  else
    response = messages[0].text;
  transform(response.begin(), response.end(), response.begin(), ::tolower);
  return response;
}

void eGardener::connectToWiFi() {
  uint8_t wifiTryCounter = 0;
  WiFiStatus status;
  do {
    if (status == WiFiStatus::WL_FAILED_COMM) {
      printf("Failed communication with Wi-Fi. Retrying..\n");
      ThisThread::sleep_for(1s);
      continue;
    }
    printf("Try %u\n", wifiTryCounter);
    wifi.connect(wifiSsid, wifiPwd);
    ThisThread::sleep_for(1s);
    wifiTryCounter++;
  } while ((status = wifi.getStatus()) != WiFiStatus::WL_CONNECTED
            && wifiTryCounter < WIFI_CONNECT_RETRIES);
         
  if (wifi.getStatus() == WiFiStatus::WL_CONNECTED) {
    printf("Wi-Fi connected succesfully\n");
    rtc.sync(wifi, TIMEZONE);
    bot.setup();
  } else {
    printf("Couldn't connect to Wi-Fi\n");
  }
}

void eGardener::setupMemoryDist() {
  uint16_t position = 0;
  // definir el tamaño no se si es medio inutil por como leo ahora, lo dejo por
  // las dudas.

  memoryDist.insert(std::pair<std::string, std::pair<uint16_t, uint8_t>>
                    ("wifi", std::pair<uint16_t, uint8_t>(position,
                     sizeof(bool) + MAX_SSID_LENGTH + MAX_PWD_LENGTH + 2)));
  position += sizeof(bool) + MAX_SSID_LENGTH + MAX_PWD_LENGTH + 2;

  memoryDist.insert(std::pair<std::string, std::pair<uint16_t, uint8_t>>
                    ("ls", std::pair<uint16_t, uint8_t>(position,
                     sizeof(bool) + sizeof(float) * 2)));
  position += sizeof(bool) + sizeof(float) * 2;

  memoryDist.insert(std::pair<std::string, std::pair<uint16_t, uint8_t>>
                    ("st", std::pair<uint16_t, uint8_t>(position,
                    sizeof(bool) * 2 + sizeof(uint8_t) + sizeof(char))));
  position += sizeof(bool) * 2 + sizeof(uint8_t) + sizeof(char);

  memoryDist.insert(std::pair<std::string, std::pair<uint16_t, uint8_t>>
                    ("ctw", std::pair<uint16_t, uint8_t>(position,
                    sizeof(bool) * 2 + sizeof(uint8_t) + sizeof(char) + sizeof(uint8_t) + sizeof(char))));
  position += sizeof(bool) * 2 + sizeof(uint8_t) + sizeof(char) + sizeof(uint8_t) + sizeof(char);

  memoryDist.insert(std::pair<std::string, std::pair<uint16_t, uint8_t>>
                    ("ctl", std::pair<uint16_t, uint8_t>(position,
                    sizeof(bool) * 2 + sizeof(uint8_t) + sizeof(char) + sizeof(uint8_t) + sizeof(char))));
  position += sizeof(bool) * 2 + sizeof(uint8_t) + sizeof(char) + sizeof(uint8_t) + sizeof(char);

  memoryDist.insert(std::make_pair("ccw", std::make_pair(position,
                    sizeof(bool) * 2 + sizeof(uint8_t) + (sizeof(char) + sizeof(uint8_t) * 2) * 4)));
  position += sizeof(bool) * 2 + sizeof(uint8_t) + (sizeof(char) + sizeof(uint8_t) * 2) * 4;

  memoryDist.insert(std::make_pair("ccl", std::make_pair(position,
                    sizeof(bool) * 2 + sizeof(uint8_t) + (sizeof(char) + sizeof(uint8_t) * 2) * 4)));
  position += sizeof(bool) * 2 + sizeof(uint8_t) + (sizeof(char) + sizeof(uint8_t) * 2) * 4;

  memoryDist.insert(std::make_pair("ur", std::make_pair(position + 10, sizeof(bool) + MAX_PWD_USER_LENGTH + 1
                                                       + sizeof(uint8_t) + sizeof(uint32_t) * MAX_NUMBER_USERS)));
  position += 10 + sizeof(bool) + MAX_PWD_USER_LENGTH + 1 + sizeof(uint8_t) + sizeof(uint32_t) * MAX_NUMBER_USERS;

  memoryDist.insert(std::pair<std::string, std::pair<uint16_t, uint8_t>>
                    ("ms", std::pair<uint16_t, uint8_t>(position,
                     sizeof(bool) + sizeof(float) * 2)));
  position += sizeof(bool) + sizeof(float) * 2;
                                                    
}


void eGardener::setControlConditions(const std::string& user_id, const std::string& body) {
  ConditionableAction * conditionable;

  if (body[0] == 'w')
    conditionable = &conditionableWater;
  else if (body[0] == 'l')
    conditionable = &conditionableLight;
  else {
    bot.sendMessage(user_id, "Control selected unknown");
    return;
  }

  if (!conditionable->setConditions(body.substr(1))) {
    bot.sendMessage(user_id, "Conditions are not correct");
    return;
  }
    
  auto address = memoryDist.find(std::string("cc") + body[0])->second;
  eeprom.write(address.first, true);

  auto conditions = conditionable->getConditions();
  uint8_t counter = 0;
  uint16_t pos_accum = 0;

  for (auto it = conditions.begin(); it != conditions.end(); it++) {
    // revisar en diccionario
    eeprom.write(address.first + sizeof(bool) * 2 + sizeof(uint8_t) + pos_accum, it->first);
    eeprom.write(address.first + sizeof(bool) * 2 + sizeof(uint8_t) + sizeof(char) + pos_accum, it->second.first);
    eeprom.write(address.first + sizeof(bool) * 2 + sizeof(uint8_t) + sizeof(char) + sizeof(uint8_t) + pos_accum, it->second.second);
    pos_accum += sizeof(char) + sizeof(uint8_t) * 2;
    counter++;
  }

  eeprom.write(address.first + sizeof(bool) * 2, counter);

  bot.sendMessage(user_id, "Control conditions for " + (body[0] + std::string(" set succesfully")));
}

void eGardener::setControlInterval(const std::string& user_id, const std::string& body) {
  // +2 por que salteo el espacio
  PeriodicAction * control;

  if (body[0] == 'w')
    control = &periodicWater;
  else if (body[0] == 'l')
    control = &periodicLight;
  else {
    bot.sendMessage(user_id, "Control selected unknown");
    return;
  }

  if (control->setIntervalAndDuration(body.substr(1), rtc.get())) {
    auto address = memoryDist.find(std::string("ct") + body[0])->second;
    eeprom.write(address.first, true);
    eeprom.write(address.first + sizeof(bool) * 2, control->getInterval());
    eeprom.write(address.first + sizeof(bool) * 2 + sizeof(uint8_t), control->getIntervalUnit());
    eeprom.write(address.first + sizeof(bool) * 2 + sizeof(uint8_t) + sizeof(char), control->getDuration());
    eeprom.write(address.first + sizeof(bool) * 2 + sizeof(uint8_t) * 2 + sizeof(char), control->getDurationUnit());
    bot.sendMessage(user_id, (std::string("Control interval ") + body[0]) + " set interval " + std::to_string(control->getInterval())
                                          + control->getIntervalUnit() + " duration " +
                                          std::to_string(control->getDuration())
                                          + control->getDurationUnit());
  } else {
    bot.sendMessage(user_id, "Time or unit are not correct");
  }
}

void eGardener::setControlIntervalStatus(const std::string& user_id, const std::string& body, bool activated) {
  PeriodicAction * control;

  if (body[0] == 'w')
    control = &periodicWater;
  else if (body[0] == 'l')
    control = &periodicLight;
  else {
    bot.sendMessage(user_id, "Control selected unknown");
    return;
  }
  control->setActivatedStatus(activated);
  auto address = memoryDist.find(std::string("ct") + body[0])->second;
  eeprom.write(address.first, true);
  eeprom.write(address.first + sizeof(bool), activated);
  bot.sendMessage(user_id, (std::string("Control interval ") + body[0]) + " " + ((activated) ? "activated" : "deactivated"));
}

void eGardener::setControlConditionsStatus(const std::string& user_id, const std::string& body, bool activated) {
  ConditionableAction * conditionable;

  if (body[0] == 'w')
    conditionable = &conditionableWater;
  else if (body[0] == 'l')
    conditionable = &conditionableLight;
  else {
    bot.sendMessage(user_id, "Control selected unknown");
    return;
  }

  conditionable->setActivatedStatus(activated);
  auto address = memoryDist.find(std::string("cc") + body[0])->second;
  eeprom.write(address.first, true);
  eeprom.write(address.first + sizeof(bool), activated);
  bot.sendMessage(user_id, (std::string("Control conditions ") + body[0]) + " " + ((activated) ? "activated" : "deactivated"));
}

void eGardener::setControlStatus(const std::string& user_id, const std::string& body, bool activated) {
  if (body[0] == 'w') {
    (activated) ? controlWater.activate() : controlWater.deactivate();
    controlWaterManually = activated;
  }
  else if (body[0] == 'l') {
    (activated) ? controlLight.activate() : controlLight.deactivate();
    controlLightManually = activated;
  }
  else {
    bot.sendMessage(user_id, "Control selected unknown");
    return;
  }

  bot.sendMessage(user_id, (std::string("Control ") + body[0]) + " " + ((activated) ? "activated" : "deactivated"));
}

void eGardener::resetWiFi() {
  wifi.setAsAP();
  printf("Set as AP\n");
}

bool eGardener::checkAndSaveNewWiFi() {
  if (wifi.getStatus() == WiFiStatus::WL_AS_AP) {
    return false;
  }

  std::string ssid, pwd;

  wifi.getSsidAndPwd(ssid, pwd);

  if (ssid.length() > MAX_SSID_LENGTH || pwd.length() > MAX_PWD_LENGTH)
    return true;

  auto address = memoryDist.find("wifi")->second;
  eeprom.write(address.first, true);
  eeprom.write(address.first + sizeof(bool), ssid);
  eeprom.write(address.first + sizeof(bool) + MAX_SSID_LENGTH + 1, pwd);

  connectToWiFi();

  return true;
}

void eGardener::activate() {
  std::vector<std::string> users;
  userRegister.getUsers(users);
  for (auto user : users) {
    sendSenseAll(user);
  }
}

void eGardener::deactivate() {
  
}