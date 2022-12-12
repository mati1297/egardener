#include <algorithm>
#include <map>
#include <vector>
#include <string>
#include "mbed.h"
#include "activable_action.h"
#include "clock.h"
#include "conditionable_action.h"

ConditionableAction::ConditionableAction(ActivableAction& action, std::vector<char> variables, bool activated):
                                        action(action), activated(), conditions(), variables(variables), executing(false) {}

bool ConditionableAction::setConditions(const std::string& conditions) {
  char parameter = 0;
  Symbol symbol = Symbol::NOTHING;
  std::string valueString;
  std::map<char, ConditionPair> result;

  for (auto c : conditions + ',') {
    if (c == ' ')
      continue;
    if (!parameter) {
      if (std::find(variables.begin(), variables.end(), c) == variables.end())
        return false;
      if (result.find(parameter) != result.end())
        return false;
      parameter = c;
      continue;
    }
    if (symbol == Symbol::NOTHING) {
      switch (c) {
        case LESS_CHAR:
          symbol = Symbol::LESS;
          break;
        case GREAT_CHAR:
          symbol = Symbol::GREAT;
          break;
        default:
          return false;
      }
      continue;
    }
    if (c == ',') {
      uint8_t value;
      char * endptr;
      if (valueString.length() > MAX_NUMBER_LENGTH || valueString.length() < 1)
        return false;
      uint16_t valueLong = strtoul(valueString.c_str(), &endptr, 10);
      // aclarar que es con punto.
      if (*endptr || *endptr == '.' || valueLong > 255)
        return false;
      value = valueLong;
      result.insert(std::pair<char, ConditionPair>(parameter, ConditionPair(symbol, value)));
      valueString.erase();
      parameter = 0;
      symbol = Symbol::NOTHING;
      continue;
    }
    if (isdigit(c) || c == '.') {
      valueString += c;
      continue;
    }
    return false;
  }

  this->conditions = result;
  return true;
}

bool ConditionableAction::setConditions(const std::map<char, ConditionPair>& conditions) {
  for (auto it = conditions.begin(); it != conditions.end(); it++) {
    if (std::find(variables.begin(), variables.end(), it->first) == variables.end())
      return false;
  }
  this->conditions = conditions;
  return true;
}

bool ConditionableAction::checkConditions(const std::map<char, uint8_t>& values) {
  for (auto it = conditions.begin(); it != conditions.end(); it++) {
    auto values_it = values.find(it->first);

    if (values_it == values.end())
      return false;

    if (it->second.first == Symbol::LESS && values_it->second > it->second.second)
      return false;
    if (it->second.first == Symbol::GREAT && values_it->second < it->second.second)
      return false;
  }

  return true;
}

void ConditionableAction::execute(const std::map<char, uint8_t>& values) {
  if (!activated)
    return;

  bool conditions = checkConditions(values);

  if (!executing && conditions) {
    action.activate();
    executing = true;
  } else if (executing && !conditions) {
    action.deactivate();
    executing = false;
  }
}

void ConditionableAction::stop() {
  if (executing)
    action.deactivate();
  executing = false;
}

void ConditionableAction::setActivatedStatus(bool activated) {
  if (!activated && executing) {
    action.deactivate();
    executing = false;
  }
  this->activated = activated;
}

bool ConditionableAction::isActivated() {
  return activated;
}

const std::map<char, ConditionableAction::ConditionPair>& ConditionableAction::getConditions() {
  return conditions;
}