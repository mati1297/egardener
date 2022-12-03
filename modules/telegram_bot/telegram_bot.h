#ifndef TELEGRAM_BOT__H 
#define TELEGRAM_BOT__H

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

    TelegramMessage(const std::string& update_id, const std::string& from_id,
                    const std::string& from_username, const std::string& from_name, 
                    const std::string& text):
                    update_id(update_id), from_id(from_id), from_username(from_username),
                    from_name(from_name), text(text) {
    // TODO validar que este bien cargado todo y arreglar forzosamente.
    // ver como eliminar constructor de struct por {}
    }
};

class TelegramBot {
private:
    WiFi& wifi;
    std::string token;
    ssize_t last_update;

    std::vector<TelegramMessage> getMessages(size_t, size_t);
    std::vector<TelegramMessage> parseJsonMessages(const Json& json);
    std::string getStringJson(const Json& json, int key);

public:
    TelegramBot(WiFi&, const std::string&);
    bool sendMessage(const std::string&, const std::string&);

    //validate correct.
    std::vector<TelegramMessage> getMessages(size_t limit=20);
};


#endif