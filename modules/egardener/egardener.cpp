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
#include "modules/wifi/wifi.h"
#include "modules/clock/clock.h"
#include "modules/memory/memory.h"
#include "modules/telegram_bot/telegram_bot.h"
#include "modules/credentials/credentials.h"
#include "modules/trh_sensor/trh_sensor.h"
#include "modules/light_sensor/light_sensor.h"
#include "modules/aux_functions/aux_functions.h"

// hacer que guarde un user y listo.
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
            lightSensor(LIGHT_SENSOR_PIN), ticker(),
            checkMessages(false) {
  setup();
}

void eGardener::execute() {
  printf("Inicializando...\n");

  uint8_t wifiTryCounter = 0;
  while (wifi.getStatus() != WiFiStatus::WL_CONNECTED
         && wifiTryCounter < WIFI_CONNECT_TRIES) {
    printf("Conectando a Wi-Fi\n");
    printf("%s\n", wifi_ssid.c_str());
    wifi.connect(wifi_ssid, wifi_pwd);
    wifiTryCounter++;
  }
  if (wifi.getStatus() == WiFiStatus::WL_CONNECTED) {
    printf("Conectado a Wi-Fi\n");
    rtc.sync(wifi, TIMEZONE);
  } else {
    printf("No se pudo conectar a Wi-Fi\n");
  }

  TelegramBot bot(wifi, TELEGRAM_BOT_TOKEN);

  // setea tickers
  ticker.attach(callback(this, &eGardener::activateCheckMessages),
                TELEGRAM_POLL_TIME);

  // validar de quien viene y guardar en memoria.
  while (true) {
    if (checkMessages) {
      std::vector<TelegramMessage> messages = bot.getMessages(
                                              MAX_AMOUNT_TELEGRAM_MESSAGES);
      for (auto message : messages) {
        // format output
        if (message.text == "/temperature") {
          bot.sendMessage(message.from_id,
                          floatToString(trhSensor.senseTemperature(), 2));
        } else if (message.text == "/humidity") {
          bot.sendMessage(message.from_id,
                          floatToString(trhSensor.senseHumidity(), 2));
        } else if (message.text == "/light") {
          float value = lightSensor.sense();
          if (value < 0)
            bot.sendMessage(message.from_id,
                            R"(You need to calibrate light sensor with
                             /calibrateLightSensor)");
          else
            bot.sendMessage(message.from_id,
                            floatToString(value * 100, 2) + "%");
        } else if (message.text == "/calibrateLightSensor") {
          float min, max;

          bot.sendMessage(message.from_id, R"(Put direct light on the sensor
                                           with your phone flash and type ok (any
                                           other text to cancel).)");
          if (getTelegramResponseForInteraction(bot) != "ok") {
            printf("Operation cancelled");
            break;
          }
          max = lightSensor.sense(true);

          bot.sendMessage(message.from_id, R"(Now put your finger on the sensor
                                             type ok (any other text to cancel).)");
          if (getTelegramResponseForInteraction(bot) != "ok") {
                      bot.sendMessage(message.from_id, "Operation cancelled");
            break;
          }
          min = lightSensor.sense(true);

          if (lightSensor.setMaxAndMin(max, min)) {
            bot.sendMessage(message.from_id, R"(Light sensor calibrated 
                                                succesfully)");
            auto address = memoryDist.find("ls")->second;
            eeprom.write(address.first, true);
            eeprom.write(address.first + sizeof(bool), max);
            eeprom.write(address.first + sizeof(bool) + sizeof(float), min);
          } else {
            bot.sendMessage(message.from_id, "An error ocurred, try again");
          }
        }
      }
      checkMessages = false;
    }
    // Por ahora, despues con timers y booleans.
  }
}

void eGardener::activateCheckMessages() {
  checkMessages = true;
}

void eGardener::setup() {
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
                    ("wifi", std::tuple<uint16_t, uint8_t>(position,
                     sizeof(bool) + MAX_SSID_LENGTH + MAX_PWD_LENGTH + 2)));
  position += sizeof(bool) + MAX_SSID_LENGTH + MAX_PWD_LENGTH;

  memoryDist.insert(std::pair<std::string, std::pair<uint16_t, uint8_t>>
                    ("ls", std::tuple<uint16_t, uint8_t>(position,
                     sizeof(bool) + sizeof(float) * 2)));
  position += sizeof(bool) + sizeof(float) * 2;
}