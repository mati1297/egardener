/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include "trh_sensor.h"

UnbufferedSerial uartUsb(USBTX, USBRX, 115200);

#define I2C_PORT2_SDA_PIN PB_9
#define I2C_PORT2_SCL_PIN PB_8
#define ADDRESS_SENSOR_RH_TEMP 0x40

TRHSensor trhSensor(I2C_PORT2_SDA_PIN, I2C_PORT2_SCL_PIN, ADDRESS_SENSOR_RH_TEMP);

InterruptIn interrupt(BUTTON1);
bool senseBool;

void sense() {
  senseBool = true;
}

int main() {
  senseBool = false;
  interrupt.rise(&sense);

  while (true) {
    if (senseBool) {
      printf("Sensed = %f\n", trhSensor.senseHumidity());
      senseBool = false;
    }
    ThisThread::sleep_for(500ms);
  }
}
