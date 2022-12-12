// Copyright 2022 Matías Charrut
// This code is licensed under MIT license (see LICENSE for details)

#include "mbed.h"
#include "control.h"

Control::Control(PinName out): out(out) {}

void Control::activate() {
  counter++;
  out = 1;
}

void Control::deactivate() {
  if (--counter == 0)
    out = 0;
}

bool Control::isActivated() {
  return out;
}
