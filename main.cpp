/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include "wifi.h"
#include "clock.h"
#include "memory.h"
#include "telegram_bot.h"
#include "credentials.h"
#include "trh_sensor.h"
#include <string>
#include <vector>
#include "aux_functions.h"

// pasarlo para que sea la de 7 bits y modificarlo adentro de la clase
#define ADDRESS_RTC 0x68
#define ADDRESS_EEPROM 0x50
#define ADDRESS_SENSOR_RH_TEMP 0x40

WiFi wifi(PD_5, PD_6, 115200);

UnbufferedSerial uartUsb(USBTX, USBRX, 115200);

InterruptIn button(BUTTON1);
DigitalOut led(LED2);

Clock rtc(PB_9, PB_8, ADDRESS_RTC);

Memory eeprom(PB_9, PB_8, ADDRESS_EEPROM);

TRHSensor sensor(PB_9, PB_8, ADDRESS_SENSOR_RH_TEMP);


void toggle_led() {
    led = !led;
}

int main() {
    /* Secuencia:
    soft reset.
    */

/*
    // mido temperatura
    sensor.start();
    sensor.write(ADDRESS_SENSOR_RH_TEMP << 1);
    sensor.write(MEAS_TEMP_NHM_CMD);
    sensor.start();
    ThisThread::sleep_for(50ms);
    while (sensor.write((ADDRESS_SENSOR_RH_TEMP << 1) | 1) != 1) {
        ThisThread::sleep_for(50ms);
        sensor.start();
    }
    uint8_t bytemsb = sensor.read(1);
    uint8_t bytelsb = sensor.read(1);
    uint8_t checksum = sensor.read(0);
    sensor.stop();

    int temp = (bytemsb << 8 | bytelsb) & TEMPERATURE_MASK;

    float temp_total = -46.85 + 175.72 * temp / (1 << 16);

    printf("temperatura: %f\n", temp_total);

    // mido humedad
    sensor.start();
    sensor.write(ADDRESS_SENSOR_RH_TEMP << 1);
    sensor.write(MEAS_RH_NHM_CMD);
    sensor.start();
    ThisThread::sleep_for(15ms);
    while (sensor.write((ADDRESS_SENSOR_RH_TEMP << 1) | 1) != 1) {
        ThisThread::sleep_for(15ms);
        sensor.start();
    }
    bytemsb = sensor.read(1);
    bytelsb = sensor.read(1);
    checksum = sensor.read(0);
    sensor.stop();

    int rh = (bytemsb << 8 | bytelsb) & RH_MASK;

    //printf("%x\n", bytemsb);
    //printf("%x\n", bytelsb);

    float rh_total = -6.0 + 125.0 * rh / (1 << 16);

    printf("rh: %f\n", rh_total);

    */
    led = 0;
    button.rise(toggle_led);
    
    printf("Inicializando...\n");

    ThisThread::sleep_for(1s);

    std::string response;

    while (wifi.getStatus() != 3) {
        printf("Conectando a WiFi..\n");
        wifi.connect(SSID, PWD);
    }

    printf("Conectado a WiFi\n");

    TelegramBot bot(wifi, TELEGRAM_BOT_TOKEN);

    // validar
    rtc.sync(wifi, "America/Argentina/Buenos_Aires");

    
    while (true) {
        std::vector<TelegramMessage> messages = bot.getMessages(100);

        for (auto message: messages) {
            printf("%s: %s\n", message.from_username.c_str(), message.text.c_str());

            if (message.text == "/time") {
                Time time = rtc.get();
                bot.sendMessage(message.from_id, time.formatTime() + " " + time.formatDate() + " " + time.formatDayWeek());
            }
            else if (message.text == "/led") {
                toggle_led();
            }
            // hacerlo bien con pasarlo a string
            else if (message.text == "/temperature") {
                bot.sendMessage(message.from_id, floatToString(sensor.senseTemperature(), 2));
            }
            else if (message.text == "/humidity") {
                bot.sendMessage(message.from_id, std::string("aca va la humedad"));
            }
        }

        ThisThread::sleep_for(1s);
    }
    
}
