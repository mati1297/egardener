// Copyright 2022 Matías Charrut
// This code is licensed under MIT license (see LICENSE for details)

#include <string>
#include <vector>
#include <algorithm>
#include "mbed.h"
#include "wifi.h"
#include "telegram_bot.h"
#include "Json.h"

TelegramBot::TelegramBot(WiFi &wifi, const std::string &token):
                         wifi(wifi), token(token), last_update(-1) {}

void TelegramBot::setup() {
  getMessages(0, 1);
}

bool TelegramBot::sendMessage(const std::string &chat_id,
                              const std::string &message) {
  std::string server = "https://api.telegram.org/bot" + token + "/sendMessage";
  std::string request = "chat_id=" + chat_id + "&text=" + message;

  std::string response = wifi.post(server, request);

  Json json(response.c_str(), response.length());

  bool result = true;

  if (!json.isValidJson())
    return false;

  if (json.tokenBooleanValue(json.findChildIndexOf(json.findKeyIndex("ok")),
                                                   result) < 0)
    return false;

  return result;
}

std::vector<TelegramMessage> TelegramBot::getMessages(size_t limit) {
  return getMessages(last_update, limit);
}

std::vector<TelegramMessage> TelegramBot::getMessages(size_t offset,
                                                      size_t limit) {
  std::string server = "https://api.telegram.org/bot" + token + "/getUpdates";
  std::string request = "offset=" + std::to_string(offset) + "&limit="
                    + std::to_string(limit);

  std::string response = wifi.post(server, request);

  std::vector<TelegramMessage> messages;

  Json json(response.c_str(), response.length(), 1000);

  if (!json.isValidJson())
    return messages;

  bool result = true;

  if (json.tokenBooleanValue(json.findChildIndexOf(json.findKeyIndex("ok")),
      result) < 0)
    return messages;

  messages = parseJsonMessages(json);

  if (!messages.empty())
    last_update = stoul(messages.back().update_id) + 1;

  return messages;
}

std::vector<TelegramMessage> TelegramBot::parseJsonMessages(const Json &json) {
  std::vector<TelegramMessage> output;

  int keyIndexResultArray = json.findChildIndexOf(json.findKeyIndex("result"));
  int resultCount = json.childCount(keyIndexResultArray);

  int keyResultI = 0;
  for (int i = 0; i < resultCount; i++) {
    keyResultI = json.findChildIndexOf(keyIndexResultArray, keyResultI);

    int keyUpdateId = json.findChildIndexOf(json.findKeyIndexIn("update_id",
                                                                keyResultI));
    std::string updateId = json.tokenString(keyUpdateId);

    ssize_t numberUpdateId = stoul(updateId);
    if (numberUpdateId < last_update)
      continue;

    int keyMessage = json.findKeyIndexIn("message", keyResultI);

    int keyFrom = json.findKeyIndexIn("from",
                                      json.findChildIndexOf(keyMessage));

    int keyFromId = json.findChildIndexOf(json.findKeyIndexIn("id",
                                          json.findChildIndexOf(keyFrom)));
    std::string fromId = json.tokenString(keyFromId);

    int keyFromName = json.findChildIndexOf(json.findKeyIndexIn("first_name",
                                            json.findChildIndexOf(keyFrom)));
    std::string fromName = json.tokenString(keyFromName);

    int keyFromUsername = json.findChildIndexOf(json.findKeyIndexIn("username",
                                                json.findChildIndexOf(
                                                  keyFrom)));
    std::string fromUserName;
    if (keyFromUsername >= 0)
      fromUserName = json.tokenString(keyFromUsername);
    else
      fromUserName = fromId;

    int keyText = json.findChildIndexOf(json.findKeyIndexIn("text",
                                        json.findChildIndexOf(keyMessage)));
    std::string text = json.tokenString(keyText);

    TelegramMessage newMessage = {updateId, fromId, fromUserName, fromName, \
                                  text};
    output.push_back(newMessage);
  }

  return output;
}

TelegramMessage::TelegramMessage(const std::string &update_id,
                                 const std::string &from_id,
                                 const std::string &from_username,
                                 const std::string &from_name,
                                 const std::string &text):
                                 update_id(update_id), from_id(from_id),
                                 from_username(from_username),
                                 from_name(from_name), text(text) {}
