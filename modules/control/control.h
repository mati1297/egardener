// Copyright 2022 Matías Charrut
// This code is licensed under MIT license (see LICENSE for details)

#ifndef MODULES_CONTROL_CONTROL_H_
#define MODULES_CONTROL_CONTROL_H_

#include <map>
#include "mbed.h"

#define LIGHT_CONTROL_DEFAULT_PIN LED1
#define WATER_CONTROL_DEFAULT_PIN LED2

class Control {
 private:
  DigitalOut light, water;

  bool activatedByConditions;
  bool deactivatedByConditions;


 public:
  Control(PinName = LIGHT_CONTROL_DEFAULT_PIN, PinName = WATER_CONTROL_DEFAULT_PIN);

  void activateLight();
  void deactivateLight();
  void activateWater();
  void deactivateWater();
};

#endif  // MODULES_CONTROL_CONTROL_H_
