// Copyright 2022 Matías Charrut
// This code is licensed under MIT license (see LICENSE for details)

#include "mbed.h"
#include "clock.h"

UnbufferedSerial uartUsb(USBTX, USBRX, 115200);

#define I2C_PORT2_SDA_PIN PB_9
#define I2C_PORT2_SCL_PIN PB_8
#define ADDRESS_RTC 0x68

Clock rtc(I2C_PORT2_SDA_PIN, I2C_PORT2_SCL_PIN, ADDRESS_RTC);

int main() {
  rtc.set(Time());

  while (true) {
    printf("%s\n", rtc.get().formatTime().c_str());
    ThisThread::sleep_for(5s);
  }
}
