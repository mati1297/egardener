#ifndef MODULES_CONDITIONABLE_ACTION_CONDITIONABLE_ACTION_H_
#define MODULES_CONDITIONABLE_ACTION_CONDITIONABLE_ACTION_H_

#include <map>
#include <vector>
#include <string>
#include "mbed.h"
#include "clock.h"
#include "activable_action.h"

#define DELIMITER ','
#define GREAT_CHAR '>'
#define LESS_CHAR '<'

#define MAX_NUMBER_LENGTH 3

// Represents actions that execute when conditions are fullfiled
class ConditionableAction {
 public:
   enum Symbol {
    NOTHING,
    GREAT,
    LESS
  };

  typedef std::pair<Symbol, uint8_t> ConditionPair;

 private:
  ActivableAction& action;
  bool activated;
  
  std::map<char, ConditionPair> conditions;
  std::vector<char> variables;

  bool executing;

  bool checkConditions(const std::map<char, uint8_t>&);

 public:
  ConditionableAction(ActivableAction&, std::vector<char>, bool = false);

  // Sets status as activated or deactivated
  void setActivatedStatus(bool);

  // Executes action if conditions are fullfiled over
  // values sensed
  void execute(const std::map<char, uint8_t>&);

  // Stops execution
  void stop();

  // Returns true is status is activated
  bool isActivated();

  // Sets conditions with dictionary
  bool setConditions(const std::map<char, ConditionPair>&);

  // Sets conditions with string
  bool setConditions(const std::string&);

  const std::map<char, ConditionPair>& getConditions();
};


#endif  // MODULES_CONDITIONABLE_ACTION_CONDITIONABLE_ACTION_H_