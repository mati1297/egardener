#ifndef EGARDENER__H
#define EGARDENER__H

#include <map>
#include <tuple>
#include <string>
#include <vector>
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

//#define MAX_SSID_LENGTH 32
//#define MAX_PWD_LENGTH 63
#define MAX_SSID_LENGTH 31 // para que entre en una pagina de la eeprom + un /0
#define MAX_PWD_LENGTH 31 // para que entre en una pagina de la eeprom.

#define WIFI_CONNECT_TRIES 3
#define TIMEZONE "America/Argentina/Buenos-Aires"
#define MAX_AMOUNT_TELEGRAM_MESSAGES 5
#define TELEGRAM_POLL_TIME 1000ms
#define TELEGRAM_POLL_TIME_WAITING 250ms

class eGardener {
private:
    std::map<std::string, std::pair<uint16_t, uint8_t>> memoryDist;
    std::string wifi_ssid, wifi_pwd;
    WiFi wifi;
    Clock rtc;
    Memory eeprom;
    TRHSensor trhSensor;
    LightSensor lightSensor;
    Ticker ticker;
    bool checkMessages;

    void setupMemoryDist();
    void setup();
    std::string getTelegramResponseForInteraction(TelegramBot &);
    void activateCheckMessages();

public:
    eGardener();
    void execute();
};

#endif