// Copyright 2022 Matías Charrut
// This code is licensed under MIT license (see LICENSE for details)

#include "mbed.h"
#include "control.h"

Control::Control(PinName light, PinName water): light(light), water(water) {}



void Control::activateLight() {
  light = 1;
}

void Control::deactivateLight() {
  light = 0;
}

void Control::activateWater() {
  water = 1;
}

void Control::deactivateWater() {
  water = 0;
}