// Copyright 2022 Matías Charrut
// This code is licensed under MIT license (see LICENSE for details)

#ifndef MODULES_CONTROL_CONTROL_H_
#define MODULES_CONTROL_CONTROL_H_

#include <map>
#include "mbed.h"
#include "periodic_action.h"

// Logical output of controls driver
class Control : public ActivableAction {
 private:
  DigitalOut out;

  uint8_t counter;

 public:
  Control(PinName);

  // Activates control
  void activate();

  // Deactivates control
  void deactivate();

  // Returns true if control is activated
  bool isActivated();
};

#endif  // MODULES_CONTROL_CONTROL_H_
