// Copyright 2022 Matías Charrut
// This code is licensed under MIT license (see LICENSE for details)

#ifndef MODULES_PERIODIC_ACTION_PERIODIC_ACTION_H_
#define MODULES_PERIODIC_ACTION_PERIODIC_ACTION_H_

#include <string>
#include "mbed.h"
#include "clock.h"
#include "activable_action.h"

#define DEFAULT_INTERVAL 30
#define DEFAULT_INTERVAL_UNIT 'd'
#define DEFAULT_DURATION 10
#define DEFAULT_DURATION_UNIT 's'

// Represents actions that execute between time intervals
class PeriodicAction {
 private:
  ActivableAction& action;
  bool activated, instant;
  uint8_t interval, duration;
  char intervalUnit, durationUnit;
  Time targetTime;

  bool executing;

  bool validateTimeAndUnit(uint8_t, char);
  void calculateAndSetNextTargetTime(const Time&, bool = true);

 public:
  PeriodicAction(ActivableAction&, bool = false, bool = true,
                 uint8_t = DEFAULT_INTERVAL, char = DEFAULT_DURATION_UNIT,
                 uint8_t = DEFAULT_DURATION, char = DEFAULT_DURATION_UNIT);

  // Sets duration of action with number and unit
  bool setDuration(uint8_t, char);

  // Sets duration of action with string
  bool setDuration(const std::string&);

  // Sets interval and duration with string
  bool setIntervalAndDuration(const std::string&, const Time&);

  // Sets interval of action with number and unit
  bool setInterval(uint8_t, char, const Time&);

  // Sets interval of action with string
  bool setInterval(const std::string&, const Time&);

  // Sets status of activation
  void setActivatedStatus(bool);

  // Sets status of activation and configures next time
  void setActivatedStatus(bool, const Time&);

  // Sets instant status and configures next time
  void setInstantStatus(bool, const Time&);

  // Sets instant status
  void setInstantStatus(bool);

  // Executes action if time condition is fullfiled
  void execute(const Time &);

  // Stops execution
  void stop();

  // Returns true if status is activated
  bool isActivated();

  // Returns interval
  uint8_t getInterval();

  // Returns interval unit
  char getIntervalUnit();

  // Returns duration
  uint8_t getDuration();

  // Returns duration unit
  char getDurationUnit();

  // Returns next time of activations
  Time getNextTime();
};


#endif  // MODULES_PERIODIC_ACTION_PERIODIC_ACTION_H_
