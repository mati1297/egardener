/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include "clock.h"

#include "credentials.h"
#include "telegram_bot.h"
#include "wifi.h"

UnbufferedSerial uartUsb(USBTX, USBRX, 115200);

#define WIFI_SERIAL_TX_PIN PD_5
#define WIFI_SERIAL_RX_PIN PD_6
#define WIFI_BAUDRATE 115200

int main() {
  WiFi wifi(WIFI_SERIAL_TX_PIN, WIFI_SERIAL_RX_PIN, WIFI_BAUDRATE);
  wifi.connect(WIFI_SSID, WIFI_PWD);
  TelegramBot bot(wifi, TELEGRAM_BOT_TOKEN);
  bot.setup();

  while (true) {
    std::vector<TelegramMessage> messages = bot.getMessages(5);
      for (auto message : messages) {
        printf("%s: %s\n", message.from_id.c_str(), message.text.c_str());
        if (message.text == "/hola")
          bot.sendMessage(message.from_id, "Hola!");
      }
    ThisThread::sleep_for(5s);
  }
}
