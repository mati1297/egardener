#ifndef MODULES_ACTIVABLE_ACTION_ACTIVABLE_ACTION_H_
#define MODULES_ACTIVABLE_ACTION_ACTIVABLE_ACTION_H_

#include "mbed.h"

class ActivableAction {
  public:
    virtual void activate();
    virtual void deactivate();
};


#endif  // MODULES_ACTIVABLE_ACTION_ACTIVABLE_ACTION_H_