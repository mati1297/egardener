// Copyright 2022 Matías Charrut
// This code is licensed under MIT license (see LICENSE for details)

#include <algorithm>
#include "mbed.h"
#include "analog_sensor.h"

AnalogSensor::AnalogSensor(PinName analog, float vref, uint8_t averagePoints):
                         analog(analog, vref), max(-1), min(-1),
                          averagePoints((averagePoints > 0) ?
                                        averagePoints : 1) {}

float AnalogSensor::sense(bool raw) {
  float rawValue = senseRaw();

  if (raw)
    return rawValue;

  if (max == -1 || min == -1)
    return -1;

  rawValue = (rawValue <= max) ? rawValue : max;
  rawValue = (rawValue >= min) ? rawValue : min;

  return (rawValue - min) / (max - min);
}

bool AnalogSensor::setMaxAndMin(float max, float min) {
  if (max <= min)
    return false;
  this->max = max;
  this->min = min;

  return true;
}

float AnalogSensor::getMax() {
  return max;
}

float AnalogSensor::getMin() {
  return min;
}

float AnalogSensor::senseRaw() {
  float total = 0;
  if (averagePoints < 1)
    averagePoints = 1;
  for (uint16_t i = 0; i < averagePoints; i++) {
    total += analog.read();
  }
  return (total / averagePoints) * -1 + 1;
}
