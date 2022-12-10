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

// hacer que guarde un user y listo.
// o hacer con contraseña?
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
            lightSensor(LIGHT_SENSOR_PIN), bot(wifi, TELEGRAM_BOT_TOKEN), control(),
            tickerCheckMessages(), tickerCheckClock(), tickerCheckControlCondition(),
            checkMessages(false), checkClock(false), senseIntervalActivated(false), checkControlConditionFlag(false),
            controlConditionActivated(true),
            senseInterval(30), 
            senseIntervalUnit('d'), senseTargetTime() {
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
        else if (message.text.substr(0, 18) == "/setsenseinterval ")
          setSenseInterval(message);
        else if (message.text == "/nextsensetime")
          sendNextSenseTime(message.from_id);
        else if (message.text.substr(0, 22) == "/setcontrolconditions ")
          setControlConditions(message);
        else
          bot.sendMessage(message.from_id, "Command unknown");
      }
      checkMessages = false;
    }
    if (checkClock) {
      Time time = rtc.get(); 
      if (senseIntervalActivated && senseTargetTime <= time) {
        senseTargetTime = calculateTargetTime(senseInterval, senseIntervalUnit);
        sendSenseAll(USER);
      }
      checkClock = false;
    }
    if (checkControlConditionFlag) {
      bool acti = checkControlConditions('w');
      printf("chequeo = %d\n", acti);
      if (acti)
        control.activateWater();
      else
        control.deactivateWater();
      checkControlConditionFlag = false;
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
  checkControlConditionFlag = true;
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
  senseIntervalActivated = activated;

  auto address = memoryDist.find("st")->second;
  eeprom.write(address.first, true);
  eeprom.write(address.first + sizeof(bool), activated);
  
  std::string state = (activated ? "activated" : "deactivated");

  bot.sendMessage(user_id, "Sense time has been " + state);
}

void eGardener::sendSenseIntervalStatus(const std::string& user_id) {
  std::string state = senseIntervalActivated ? "activated" : "deactivated";
  bot.sendMessage(user_id, "Sense interval is " + state +
                  "\nSense interval is " + to_string(senseInterval) +
                  senseIntervalUnit);
}

void eGardener::sendNextSenseTime(const std::string& user_id) {
  bot.sendMessage(user_id, "Next sense at " + senseTargetTime.formatTime()
                          + " " + senseTargetTime.formatDate());
}

void eGardener::setSenseInterval(const TelegramMessage& message) {
  size_t idx = 0;

  std::string parameter = message.text.substr(18);

  char *endptr;

  uint8_t newInterval = std::strtoul(parameter.c_str(), &endptr, 10);
  char newUnit = *endptr;

  // Crear macros
  if ((newInterval < 60 && newInterval >= 10 && newUnit == 's') ||
      (newInterval < 60 && newInterval > 0 && newUnit == 'm') ||
      (newInterval < 24 && newInterval > 0 && newUnit == 'h') ||
      (newInterval <= 30 && newInterval > 0 && newUnit == 'd')) {
    senseInterval = newInterval;
    senseIntervalUnit = newUnit;
    senseTargetTime = calculateTargetTime(senseInterval, senseIntervalUnit);

    auto address = memoryDist.find("st")->second;
    eeprom.write(address.first, true);
    eeprom.write(address.first + sizeof(bool) * 2, senseInterval);
    eeprom.write(address.first + sizeof(bool) * 2 + sizeof(uint8_t), senseIntervalUnit);

    bot.sendMessage(message.from_id, "Sense time set " + std::to_string(senseInterval)
                                     + senseIntervalUnit);
  } else {
    bot.sendMessage(message.from_id, "El intervalo o la unidad no son correctos");
  }
  
}

Time eGardener::calculateTargetTime(uint8_t interval, char unit) {
  Time result = rtc.get();
  if (unit == 's')
    result.addSeconds(interval);
  else if (unit == 'm')
    result.addMinutes(interval);
  else if (unit == 'h')
    result.addHours(interval);
  else if (unit == 'd')
    result.addDays(interval);
  else
    result = Time(0, 0, 0, 0, 0, 99);

  return result;
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
    senseInterval = interval;
    senseIntervalActivated = activated;
    senseIntervalUnit = intervalUnit;
  }
  senseTargetTime = calculateTargetTime(senseInterval, senseIntervalUnit);


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
}

void eGardener::setControlConditions(const TelegramMessage& message) {
  std::string toParse = message.text.substr(22);
  char variableToSet;

  printf("%s\n", toParse.c_str());

  int i;

  for (i = 0; i < toParse.length(); i++) {
    if (toParse[i] == ' ')
      continue;
    if (toParse[i] == 'w') {
      variableToSet = 'w';
      break;
    }
    else if (toParse[i] == 'l') {
      variableToSet = 'l';
      break;
    }
    else {
      bot.sendMessage(message.from_id, "Wrong variable to set");
      return;
    }
  }

  auto conditions = parseVariableConditions(toParse.substr(i+1));

  if (conditions.empty()) {
    bot.sendMessage(message.from_id, "Wrong conditions input");
    return;
  }

  if (variableToSet == 'w')
    controlConditionsWater = conditions;
  else if (variableToSet == 'l')
    controlConditionsLight = conditions;

  bot.sendMessage(message.from_id, "Control conditions for " + (variableToSet + std::string(" set succesfully")));
}

bool eGardener::checkControlConditions(char variable) {
  bool activate = true;
  std::map<char, controlConditionPair> conditions;

  if (variable == 'w')
    conditions = controlConditionsWater;
  else if (variable == 'l')
    conditions = controlConditionsLight;
  else
    return false;

  if (conditions.empty())
    return false;
    

  for (auto it = conditions.begin(); it != conditions.end(); it++) {
    float variable;
    printf("%c: %d, %u\n", it->first, it->second.first, it->second.second);
    switch (it->first) {
      case 'l':
        // validar si esta calibrado
        variable = lightSensor.sense() * 100;
        printf("variable chequeada = %f", variable);
        break;
      case 't':
        variable = trhSensor.senseTemperature();
        break;
      case 'h':
        variable = trhSensor.senseHumidity();
        break;
    }
    if (it->second.first == ControlSymbol::LESS)
      activate = activate && variable <= it->second.second;
    else if (it->second.first == ControlSymbol::GREAT)
      activate = activate && variable >= it->second.second;
  }

  return activate;
}

std::map<char, eGardener::controlConditionPair>eGardener::parseVariableConditions(const std::string& input) {
  char parameter = 0;
  ControlSymbol symbol = ControlSymbol::NOTHING;
  std::string valueString;
  std::map<char, controlConditionPair> result;

  for (auto c : input + ',') {
    if (c == ' ')
      continue;
    if (!parameter) {
      if (result.find(parameter) != result.end())
        return std::map<char, controlConditionPair>();
      if (c == CONTROL_TEMPERATURE_CHAR || c == CONTROL_LIGHT_CHAR || c == CONTROL_HUMIDITY_CHAR) {
        parameter = c;
        continue;
      }
    }
    if (symbol == ControlSymbol::NOTHING) {
      switch (c) {
        case CONTROL_LESS_CHAR:
          symbol = ControlSymbol::LESS;
          break;
        case CONTROL_GREAT_CHAR:
          symbol = ControlSymbol::GREAT;
          break;
        default:
          return std::map<char, controlConditionPair>();
      }
      continue;
    }
    if (c == ',') {
      uint8_t value;
      char * endptr;
      if (valueString.length() > CONTROL_MAX_NUMBER_LENGTH || valueString.length() < 1)
        return std::map<char, controlConditionPair>();
      uint16_t valueLong = strtoul(valueString.c_str(), &endptr, 10);
      // aclarar que es con punto.
      if (*endptr || *endptr == '.' || valueLong > 255)
        return std::map<char, controlConditionPair>();
      value = valueLong;
      result.insert(std::pair<char, controlConditionPair>(parameter, controlConditionPair(symbol, value)));
      valueString.erase();
      parameter = 0;
      symbol = ControlSymbol::NOTHING;
      continue;
    }
    if (isdigit(c) || c == '.') {
      valueString += c;
      continue;
    }
    return std::map<char, controlConditionPair>();
  }
  return result;
}