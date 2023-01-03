// Copyright 2022 Matías Charrut
// This code is licensed under MIT license (see LICENSE for details)

#ifndef MODULES_ACTIVABLE_ACTION_ACTIVABLE_ACTION_H_
#define MODULES_ACTIVABLE_ACTION_ACTIVABLE_ACTION_H_

#include "mbed.h"

// Abstract class that represent activable actions
class ActivableAction {
 public:
    // Abstract method for activate action
    virtual void activate();

    // Abstract method form deactivate action
    virtual void deactivate();
};


#endif  // MODULES_ACTIVABLE_ACTION_ACTIVABLE_ACTION_H_
