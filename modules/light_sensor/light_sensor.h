// Copyright 2022 Matías Charrut
// This code is licensed under MIT license (see LICENSE for details)

#ifndef MODULES_LIGHT_SENSOR_LIGHT_SENSOR_H_
#define MODULES_LIGHT_SENSOR_LIGHT_SENSOR_H_

#include "mbed.h"

#define VOLTAGE_REF_DEFAULT 3.3
#define AVERAGE_POINTS_DEFAULT 10

class LightSensor {
 private:
  AnalogIn analog;
  float max, min;
  uint8_t averagePoints;

  float senseRaw();

 public:
  LightSensor(PinName, float vref = VOLTAGE_REF_DEFAULT,
              uint8_t = AVERAGE_POINTS_DEFAULT);
  void calibrateMax();
  void calibrateMin();

  float sense(bool raw = false);

  float getMax();
  float getMin();
  bool setMaxAndMin(float, float);
};

#endif  // MODULES_LIGHT_SENSOR_LIGHT_SENSOR_H_
