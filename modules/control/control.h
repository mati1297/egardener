// Copyright 2022 Matías Charrut
// This code is licensed under MIT license (see LICENSE for details)

#ifndef MODULES_CONTROL_CONTROL_H_
#define MODULES_CONTROL_CONTROL_H_

#include <map>
#include "mbed.h"
#include "periodic_action.h"

class Control : public ActivableAction {
 private:
  DigitalOut out;

  uint8_t counter;

 public:
  Control(PinName);

  void activate();
  void deactivate();
};

#endif  // MODULES_CONTROL_CONTROL_H_
