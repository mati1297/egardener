/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include "memory.h"
#include "egardener.h"


#define I2C_PORT2_SDA_PIN PB_9
#define I2C_PORT2_SCL_PIN PB_8 

#define WIFI_SERIAL_TX_PIN PD_5
#define WIFI_SERIAL_RX_PIN PD_6
#define WIFI_BAUDRATE 115200

#define LIGHT_SENSOR_PIN A1

#define ADDRESS_RTC 0x68
#define ADDRESS_EEPROM 0x50
#define ADDRESS_SENSOR_RH_TEMP 0x40

UnbufferedSerial uartUsb(USBTX, USBRX, 115200);

int main() {
    ThisThread::sleep_for(1s);

    eGardener eGardener;

    eGardener.execute();
}
