// Copyright 2022 Matías Charrut
// This code is licensed under MIT license (see LICENSE for details)

#ifndef MODULES_ANALOG_SENSOR_ANALOG_SENSOR_H_
#define MODULES_ANALOG_SENSOR_ANALOG_SENSOR_H_

#include "mbed.h"

#define VOLTAGE_REF_DEFAULT 3.3
#define AVERAGE_POINTS_DEFAULT 10


// Analog sensor driver
class AnalogSensor {
 private:
  AnalogIn analog;
  float max, min;
  uint8_t averagePoints;

  float senseRaw();

 public:
  AnalogSensor(PinName, float vref = VOLTAGE_REF_DEFAULT,
              uint8_t = AVERAGE_POINTS_DEFAULT);

  // Returns sensor value. If raw is false, then the value
  // is calculated in function of max and min.
  float sense(bool raw = false);

  // Returns calibrated max value.
  float getMax();

  // Returns calibrated min value.
  float getMin();

  // Validates and sets maximum and minimum values.
  bool setMaxAndMin(float, float);
};

#endif  // MODULES_ANALOG_SENSOR_ANALOG_SENSOR_H_
