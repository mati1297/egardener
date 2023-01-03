// Copyright 2022 Matías Charrut
// This code is licensed under MIT license (see LICENSE for details)

#include "mbed.h"
#include "analog_sensor.h"

UnbufferedSerial uartUsb(USBTX, USBRX, 115200);


#define MOISTURE_SENSOR_PIN A0

AnalogSensor moistureSensor(MOISTURE_SENSOR_PIN);

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
      printf("Sensed = %f\n", moistureSensor.sense(true));
      senseBool = false;
    }
    ThisThread::sleep_for(500ms);
  }
}
