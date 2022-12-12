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
#include "light_sensor.h"
#include "aux_functions.h"
#include "control.h"
#include "conditionable_action.h"
#include "activable_action.h"

// hacer que guarde un user y listo.
// o hacer con contraseña?

// TODO(matiascharrut) guardar.
eGardener::eGardener(): memoryDist(),
            wifi(WIFI_SERIAL_TX_PIN, WIFI_SERIAL_RX_PIN,
               WIFI_BAUDRATE),
            wifi_ssid(), wifi_pwd(),
            rtc(I2C_PORT2_SDA_PIN, I2C_PORT2_SCL_PIN,
              ADDRESS_RTC),
            eeprom(I2C_PORT2_SDA_PIN, I2C_PORT2_SCL_PIN,
                 ADDRESS_EEPROM),
            trhSensor(I2C_PORT2_SDA_PIN, I2C_PORT2_SCL_PIN,
                  ADDRESS_SENSOR_RH_TEMP),
            lightSensor(LIGHT_SENSOR_PIN), bot(wifi, TELEGRAM_BOT_TOKEN), controlLight(LIGHT_PIN), controlWater(WATER_PIN),
            tickerCheckMessages(), tickerCheckClock(), tickerCheckControlCondition(),
            checkMessages(false), checkClock(false), checkControlCondition(false),
            periodicSense(*this), periodicWater(controlWater, false, false), periodicLight(controlLight, false, false),
            conditionableWater(controlWater, std::vector<char>{CONTROL_HUMIDITY_CHAR, CONTROL_LIGHT_CHAR, CONTROL_TEMPERATURE_CHAR}),
            conditionableLight(controlLight, std::vector<char>{CONTROL_HUMIDITY_CHAR, CONTROL_LIGHT_CHAR, CONTROL_TEMPERATURE_CHAR}) {
  setup();
}

void eGardener::execute() {
  // validar de quien viene y guardar en memoria.
  while (true) {
    if (checkMessages) {
      std::vector<TelegramMessage> messages = bot.getMessages(
                                              MAX_AMOUNT_TELEGRAM_MESSAGES);
      for (auto message : messages) {
        // format output
        size_t firstSpacePos = message.text.find_first_of(' ');
        if (message.text == "/start")
          sendWelcomeMessage(message.from_id);
        else if (message.text == "/temperature")
          sendTemperature(message.from_id);
        else if (message.text == "/humidity")
          sendHumidity(message.from_id);
        else if (message.text == "/light")
          sendLight(message.from_id);
        else if (message.text == "/calibratelightsensor")
          calibrateLightSensor(message.from_id);
        else if(message.text == "/senseall")
          sendSenseAll(message.from_id);
        else if (message.text == "/activatesenseinterval")
          setSenseIntervalActivated(message.from_id, true); // si se activa queda desde el ultimo.
        else if (message.text == "/deactivatesenseinterval")
          setSenseIntervalActivated(message.from_id, false);
        else if (message.text == "/senseintervalstatus")
          sendSenseIntervalStatus(message.from_id);
        else if (firstSpacePos != std::string::npos && message.text.substr(0, firstSpacePos) == "/setsenseinterval")
          setSenseInterval(message.from_id, message.text.substr(firstSpacePos+1));
        else if (message.text == "/nextsensetime")
          sendNextSenseTime(message.from_id);
        else if (firstSpacePos != std::string::npos && message.text.substr(0, firstSpacePos) == "/activatecontrolconditions")
          setControlConditionsStatus(message.from_id, message.text.substr(firstSpacePos+1), true);
        else if (firstSpacePos != std::string::npos && message.text.substr(0, firstSpacePos) == "/deactivatecontrolconditions")
          setControlConditionsStatus(message.from_id, message.text.substr(firstSpacePos+1), false);
        else if (firstSpacePos != std::string::npos && message.text.substr(0, firstSpacePos) == "/setcontrolconditions")
          setControlConditions(message.from_id, message.text.substr(firstSpacePos+1));
        else if (firstSpacePos != std::string::npos && message.text.substr(0, firstSpacePos) == "/setcontrolinterval")
          setControlInterval(message.from_id, message.text.substr(firstSpacePos+1));
        else if (firstSpacePos != std::string::npos && message.text.substr(0, firstSpacePos) == "/activatecontrolinterval")
          setControlIntervalStatus(message.from_id, message.text.substr(firstSpacePos+1), true);
        else if (firstSpacePos != std::string::npos && message.text.substr(0, firstSpacePos) == "/deactivatecontrolinterval")
          setControlIntervalStatus(message.from_id, message.text.substr(firstSpacePos+1), false);
        // TODO(matiascharrut) show control conditions an interval with status. (Y estatus de si esta prendido).
        else if (firstSpacePos != std::string::npos && message.text.substr(0, firstSpacePos) == "/nextcontroltime")
          sendNextControlTime(message.from_id, message.text.substr(firstSpacePos+1));
        else if (firstSpacePos != std::string::npos && message.text.substr(0, firstSpacePos) == "/activatecontrol")
          setControlStatus(message.from_id, message.text.substr(firstSpacePos+1), true);
        else if (firstSpacePos != std::string::npos && message.text.substr(0, firstSpacePos) == "/deactivatecontrol")
          setControlStatus(message.from_id, message.text.substr(firstSpacePos+1), false);
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
                                     {CONTROL_LIGHT_CHAR, lightSensor.sense() * 100}};
      conditionableWater.execute(values);
      conditionableLight.execute(values);
      checkControlCondition = false;
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

void eGardener::sendWelcomeMessage(const std::string& user_id) {
  bot.sendMessage(user_id, "Welcome to eGardener!");
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

void eGardener::sendLight(const std::string& user_id) {
  Time time = rtc.get();
  float value = lightSensor.sense();
  if (value < 0)
    bot.sendMessage(user_id,
                    R"(You need to calibrate light sensor with
                      /calibrateLightSensor)");
  else
    bot.sendMessage(user_id, "[" + time.formatTime() + " " + time.formatDate() + "] " +
                    LIGHT_EMOJI + (" " + 
                    floatToString(value * 100, 2)) + "%");
}

void eGardener::sendSenseAll(const std::string& user_id) {
  float value = lightSensor.sense();
  if (value < 0) {
    bot.sendMessage(user_id,
                    R"(You need to calibrate light sensor with
                      /calibrateLightSensor)");
  } else {
    Time time = rtc.get();
    std::string messageTime = "[" + time.formatTime() + " " + time.formatDate() + "]";
    std::string temp = TEMPERATURE_EMOJI + (" " +
                  floatToString(trhSensor.senseTemperature(), 2)) + 
                  "°C";
    std::string humidity = HUMIDITY_EMOJI + (" " + 
                  floatToString(trhSensor.senseHumidity(), 2)) + 
                  "% ";
    std::string light = LIGHT_EMOJI + (" " +
                  floatToString(value * 100, 2)) + "%";
    bot.sendMessage(user_id, messageTime + "\n" + temp + "\n" + humidity + "\n" + light);
  }
}

void eGardener::calibrateLightSensor(const std::string& user_id) {
  float min, max;

  bot.sendMessage(user_id, R"(Put direct light on the sensor
                                    with your phone flash and type ok (any
                                    other text to cancel).)");
  if (getTelegramResponseForInteraction(bot) != "ok") {
    printf("Operation cancelled");
    return;
  }
  max = lightSensor.sense(true);

  bot.sendMessage(user_id, R"(Now put your finger on the sensor
                                      type ok (any other text to cancel).)");
  if (getTelegramResponseForInteraction(bot) != "ok") {
              bot.sendMessage(user_id, "Operation cancelled");
    return;
  }
  min = lightSensor.sense(true);

  if (lightSensor.setMaxAndMin(max, min)) {
    bot.sendMessage(user_id, R"(Light sensor calibrated 
                                        succesfully)");
    auto address = memoryDist.find("ls")->second;
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
  

  bot.sendMessage(user_id, "Sense time has been " + state);
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

void eGardener::setSenseInterval(const std::string& user_id, const std::string& body) {
  // Crear macros
  if (periodicSense.setInterval(body, rtc.get())) {
    auto address = memoryDist.find("st")->second;
    eeprom.write(address.first, true);
    eeprom.write(address.first + sizeof(bool) * 2, periodicSense.getInterval());
    eeprom.write(address.first + sizeof(bool) * 2 + sizeof(uint8_t), periodicSense.getIntervalUnit());

    bot.sendMessage(user_id, "Sense time set " + std::to_string(periodicSense.getInterval())
                                     + periodicSense.getIntervalUnit());
  } else {
    bot.sendMessage(user_id, "El intervalo o la unidad no son correctos");
  }
}

void eGardener::setup() {
  printf("Inicializando...\n");
  printf("Cargando datos de EEPROM...\n");
  setupMemoryDist();
  bool there;
  // leo wifi.
  auto address = memoryDist.find("wifi")->second;
  // validar que lo encuentre?
  eeprom.read(address.first, there);
  if (there) {
    eeprom.read(address.first + sizeof(bool), wifi_ssid);
    eeprom.read(address.first + sizeof(bool) + MAX_SSID_LENGTH + 1, wifi_pwd);
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

  printf("Conectando a Wi-Fi...\n");

  uint8_t wifiTryCounter = 0;
  WiFiStatus status;
  while ((status = wifi.getStatus()) != WiFiStatus::WL_CONNECTED
         && wifiTryCounter < WIFI_CONNECT_TRIES) {
    if (status == WiFiStatus::WL_FAILED_COMM) {
      printf("Failed comm\n");
      ThisThread::sleep_for(1s);
      continue;
    }
    printf("Intento %u\n", wifiTryCounter);
    wifi.connect(wifi_ssid, wifi_pwd);
    wifiTryCounter++;
  }
  if (wifi.getStatus() == WiFiStatus::WL_CONNECTED) {
    printf("Conectado a Wi-Fi correctamente\n");
    rtc.sync(wifi, TIMEZONE);
    bot.setup();
  } else {
    printf("No se pudo conectar a Wi-Fi\n");
  }

  printf("Configurando tickers...\n");
  // setea tickers
  tickerCheckMessages.attach(callback(this, &eGardener::activateCheckMessages),
                TELEGRAM_POLL_TIME);
  tickerCheckClock.attach(callback(this, &eGardener::activateCheckClock),
                CLOCK_POLL_TIME);
  tickerCheckControlCondition.attach(callback(this, &eGardener::activateCheckControlCondition),
                CONTROL_CONDITION_POLL_TIME);

  printf("Listo!\n");
}

// devuelve todo en minusculas.
std::string eGardener::getTelegramResponseForInteraction(TelegramBot& bot) {
  std::vector<TelegramMessage> messages;
  while (messages.empty()) {
    messages = bot.getMessages(1);
    ThisThread::sleep_for(TELEGRAM_POLL_TIME_WAITING);
  }
  std::string response = messages[0].text;
  transform(response.begin(), response.end(), response.begin(), ::tolower);
  return response;
}


void eGardener::setupMemoryDist() {
  uint16_t position = 0;
  // definir el tamaño no se si es medio inutil por como leo ahora, lo dejo por
  // las dudas.

  memoryDist.insert(std::pair<std::string, std::pair<uint16_t, uint8_t>>
                    ("wifi", std::pair<uint16_t, uint8_t>(position,
                     sizeof(bool) + MAX_SSID_LENGTH + MAX_PWD_LENGTH + 2)));
  position += sizeof(bool) + MAX_SSID_LENGTH + MAX_PWD_LENGTH;

  memoryDist.insert(std::pair<std::string, std::pair<uint16_t, uint8_t>>
                    ("ls", std::pair<uint16_t, uint8_t>(position,
                     sizeof(bool) + sizeof(float) * 2)));
  position += sizeof(bool) + sizeof(float) * 2;

  memoryDist.insert(std::pair<std::string, std::pair<uint16_t, uint8_t>>
                    ("st", std::pair<uint16_t, uint8_t>(position,
                    sizeof(bool) * 2+ sizeof(uint8_t) + sizeof(char))));
  position = sizeof(bool) * 2 + sizeof(uint8_t) + sizeof(char);

  memoryDist.insert(std::make_pair("ccw", std::make_pair(position, sizeof(bool) * 2 + 
                    (sizeof(char) + sizeof(uint8_t)) * 4)));
  position = sizeof(bool) * 2 + (sizeof(char) + sizeof(uint8_t)) * 4;

  memoryDist.insert(std::make_pair("ccl", std::make_pair(position, sizeof(bool) * 2 + 
                    (sizeof(char) + sizeof(uint8_t)) * 4)));
  position = sizeof(bool) * 2 + (sizeof(char) + sizeof(uint8_t)) * 4;

  memoryDist.insert(std::make_pair("ctw", std::make_pair(position, sizeof(bool) + 
                    sizeof(uint8_t) + sizeof(char) + sizeof(uint8_t) + sizeof(char))));
  position = sizeof(bool) + 
                    sizeof(uint8_t) + sizeof(char) + sizeof(uint8_t) + sizeof(char);

  memoryDist.insert(std::make_pair("ctl", std::make_pair(position, sizeof(bool) * 2 + 
                    sizeof(uint8_t) + sizeof(char) + sizeof(uint8_t) + sizeof(char))));    
  position = sizeof(bool) * 2 + 
                    sizeof(uint8_t) + sizeof(char) + sizeof(uint8_t) + sizeof(char);        
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

  conditionable->setConditions(body.substr(1));

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
    // TODO(matiascharrut) GUARDAR!
    bot.sendMessage(user_id, (std::string("Control time ") + body[0]) + " set interval " + std::to_string(control->getInterval())
                                          + control->getIntervalUnit() + " duration " +
                                          std::to_string(control->getDuration())
                                          + control->getDurationUnit());
  } else {
    bot.sendMessage(user_id, "Time or unit are not correct");
  }
}

void eGardener::setControlIntervalStatus(const std::string& user_id, const std::string& body, bool activated) {
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
  control->setActivatedStatus(activated);
  // TODO(matiascharrut) GUARDAR!
  bot.sendMessage(user_id, (std::string("Control time ") + body[0]) + " " + ((activated) ? "activated" : "deactivated"));
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
  // TODO(matiascharrut) GUARDAR!
  bot.sendMessage(user_id, (std::string("Control conditions ") + body[0]) + " " + ((activated) ? "activated" : "deactivated"));
}

void eGardener::setControlStatus(const std::string& user_id, const std::string& body, bool activated) {
  Control * control;

  if (body[0] == 'w')
    control = &controlWater;
  else if (body[0] == 'l')
    control = &controlLight;
  else {
    bot.sendMessage(user_id, "Control selected unknown");
    return;
  }
  (activated) ? control->activate() : control->deactivate();
  // TODO(matiascharrut) GUARDAR!
  bot.sendMessage(user_id, (std::string("Control ") + body[0]) + " " + ((activated) ? "activated" : "deactivated"));
}

void eGardener::activate() {
  sendSenseAll(USER);
}

void eGardener::deactivate() {
  
}