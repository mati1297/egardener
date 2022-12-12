#ifndef MODULES_PERIODIC_ACTION_PERIODIC_ACTION_H_
#define MODULES_PERIODIC_ACTION_PERIODIC_ACTION_H_

#include "mbed.h"
#include "clock.h"
#include "activable_action.h"

#define DEFAULT_INTERVAL 30
#define DEFAULT_INTERVAL_UNIT 'd'
#define DEFAULT_DURATION 10
#define DEFAULT_DURATION_UNIT 's'

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
  PeriodicAction(ActivableAction&, bool = false, bool = true, uint8_t = DEFAULT_INTERVAL, char = DEFAULT_DURATION_UNIT,
                 uint8_t = DEFAULT_DURATION, char = DEFAULT_DURATION_UNIT);
                 
  bool setDuration(uint8_t, char);
  bool setIntervalAndDuration(const std::string&, const Time&);
  bool setDuration(const std::string&);
  bool setInterval(uint8_t, char, const Time&);
  bool setInterval(const std::string&, const Time&);
  void setActivatedStatus(bool);
  void setActivatedStatus(bool, const Time&);
  void setInstantStatus(bool, const Time&);
  void setInstantStatus(bool);
  void execute(const Time &);
  void stop();

  bool isActivated();
  uint8_t getInterval();
  char getIntervalUnit();
  uint8_t getDuration();
  char getDurationUnit();
  Time getNextTime();
};


#endif  // MODULES_PERIODIC_ACTION_PERIODIC_ACTION_H_