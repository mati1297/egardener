#include "mbed.h"
#include "activable_action.h"
#include "clock.h"
#include "periodic_action.h"


PeriodicAction::PeriodicAction(ActivableAction& action, bool activated, bool instant, uint8_t interval, char intervalUnit,
               uint8_t duration, char durationUnit): action(action), activated(activated), instant(instant),
               interval(interval), duration(duration), intervalUnit(intervalUnit), durationUnit(durationUnit),
               executing(false) {}

bool PeriodicAction::validateTimeAndUnit(uint8_t time, char unit) {
  return ((time < 60 && time >= 10 && unit == 's') ||
          (time < 60 && time > 0 && unit == 'm') ||
          (time < 24 && time > 0 && unit == 'h') ||
          (time <= 30 && time > 0 && unit == 'd'));
}

void PeriodicAction::execute(const Time& now) {
  if (!activated || now < targetTime)
    return;

  if (!executing) {
    action.activate();
    if (instant)
      calculateAndSetNextTargetTime(now);
    else {
      calculateAndSetNextTargetTime(now, false);
      executing = true;
    }
  } else {
    action.deactivate();
    calculateAndSetNextTargetTime(now);
    executing = false;
  }
}

void PeriodicAction::stop() {
  if (executing)
    action.deactivate();
  executing = false;
}

void PeriodicAction::calculateAndSetNextTargetTime(const Time& now, bool by_interval) {
  uint8_t time = (by_interval) ? interval : duration;
  char unit = (by_interval) ? intervalUnit : durationUnit;

  Time newTargetTime = now;

  if (unit == 's')
    newTargetTime.addSeconds(time);
  else if (unit == 'm')
    newTargetTime.addMinutes(time);
  else if (unit == 'h')
    newTargetTime.addHours(time);
  else if (unit == 'd')
    newTargetTime.addDays(time);

  targetTime = newTargetTime;
}

void PeriodicAction::setActivatedStatus(bool activated, const Time& now) {
  if (activated)
    calculateAndSetNextTargetTime(now);
  setActivatedStatus(activated);
}

void PeriodicAction::setActivatedStatus(bool activated) {
  if (!activated && executing) {
    action.deactivate();
    executing = false;
  }
  this->activated = activated;
}

void PeriodicAction::setInstantStatus(bool instant, const Time& now) {
  if (!instant)
    calculateAndSetNextTargetTime(now);
  setInstantStatus(instant);
}

void PeriodicAction::setInstantStatus(bool instant) {
  this->instant = instant;
}

bool PeriodicAction::setIntervalAndDuration(const std::string& body, const Time& now) {
  size_t spacePos = body.find_first_of(' ', 2);

  auto str1 = body.substr(0, spacePos);
  auto str2 = body.substr(spacePos + 1);

  char * endptr;
  uint8_t timeInterval = std::strtoul(str1.c_str(), &endptr, 10);
  char unitInterval = *endptr;

  uint8_t timeDuration = std::strtoul(str2.c_str(), &endptr, 10);
  char unitDuration = *endptr;

  bool intervalOk = validateTimeAndUnit(timeInterval, unitInterval);
  bool durationOk = validateTimeAndUnit(timeDuration, unitDuration);

  if (intervalOk && durationOk) {
    setInterval(timeInterval, unitInterval, now);
    setDuration(timeDuration, unitDuration);
    return true;
  }
  return false;
}

bool PeriodicAction::setDuration(uint8_t duration, char durationUnit) {
  if (!validateTimeAndUnit(duration, durationUnit))
    return false;

  if (executing) {
    action.deactivate();
    executing = false;
  }

  this->duration = duration;
  this->durationUnit = durationUnit;

  return true;
}

bool PeriodicAction::setDuration(const std::string& body) {
  char * endptr;
  uint8_t time = std::strtoul(body.c_str(), & endptr, 10);
  char unit = *endptr;

  return setDuration(time, unit);
}

bool PeriodicAction::setInterval(uint8_t interval, char intervalUnit, const Time& now) {
  if (!validateTimeAndUnit(interval, intervalUnit))
    return false;

  this->interval = interval;
  this->intervalUnit = intervalUnit;

    calculateAndSetNextTargetTime(now);

  return true;
}

bool PeriodicAction::setInterval(const std::string& body, const Time& now) {
  char * endptr;
  uint8_t time = std::strtoul(body.c_str(), & endptr, 10);
  char unit = *endptr;

  return setInterval(time, unit, now);
}

bool PeriodicAction::isActivated() {
  return activated;
}

uint8_t PeriodicAction::getInterval() {
  return interval;
}

char PeriodicAction::getIntervalUnit() {
  return intervalUnit;
}

uint8_t PeriodicAction::getDuration() {
  return duration;
}

char PeriodicAction::getDurationUnit() {
  return durationUnit;
}

Time PeriodicAction::getNextTime() {
  return targetTime;
}