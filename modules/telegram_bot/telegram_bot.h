// Copyright 2022 Matías Charrut
// This code is licensed under MIT license (see LICENSE for details)

#ifndef MODULES_TELEGRAM_BOT_TELEGRAM_BOT_H_
#define MODULES_TELEGRAM_BOT_TELEGRAM_BOT_H_

#include <vector>
#include <string>
#include "mbed.h"
#include "wifi.h"
#include "Json.h"

struct TelegramMessage {
  const std::string update_id;
  const std::string from_id;
  const std::string from_username;
  const std::string from_name;
  const std::string text;

  TelegramMessage(const std::string&, const std::string &, const std::string&,
                  const std::string&, const std::string&);
};

class TelegramBot {
 private:
  WiFi &wifi;
  std::string token;
  ssize_t last_update;

  std::vector<TelegramMessage> getMessages(size_t, size_t);
  std::vector<TelegramMessage> parseJsonMessages(const Json &json);
  std::string getStringJson(const Json &json, int key);

 public:
  TelegramBot(WiFi &, const std::string &);
  bool sendMessage(const std::string &, const std::string &);

  // validate correct.
  std::vector<TelegramMessage> getMessages(size_t limit = 20);
};

#endif  // MODULES_TELEGRAM_BOT_TELEGRAM_BOT_H_
