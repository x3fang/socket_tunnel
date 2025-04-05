#ifndef FLITER_H
#define FLITER_H
#include <map>
#include <cstring>
#include <bitset>
#include <variant>
#include <memory>
class Fliter
{
public:
      static const int optNum = 6;

private:
      using VariantIntOrString = std::variant<int, std::string>;
      struct ruleValue
      {
            bool empty = true;
            bool useRule = true;
            VariantIntOrString value;
            int ruleUsedOpt;
            ruleValue() = default;
            ruleValue(const int ruleUsedOpt_) : ruleUsedOpt(ruleUsedOpt_), empty(false) {}
            ruleValue(const int ruleUsedOpt_, VariantIntOrString value_) : ruleUsedOpt(ruleUsedOpt_), empty(false), value(std::move(value_)) {}
            ruleValue &operator=(ruleValue &other)
            {
                  this->ruleUsedOpt = other.ruleUsedOpt;
                  this->value = other.value;
                  this->useRule = other.useRule;
                  return *this;
            }
            bool operator!() const
            {
                  return !empty;
            }
      };
      struct ruleNode
      {
            std::string id;
            std::bitset<optNum> UsedOpt;
            ruleValue rule;
            ruleNode(const std::bitset<optNum> UsedOpt_) : next(nullptr), UsedOpt(UsedOpt_) {}
            bool operator!() const
            {
                  return !rule;
            }
            std::shared_ptr<ruleNode> next;
      };
      //< <= > >= == !=
      // 0 1 2 3 4 5
      std::map<std::string, std::pair<bool, std::shared_ptr<ruleNode>>> ruleTypeList;

public:
      /*
      if this ruleType's typeSupportOpt [2~5] set 1,use int to save value else use string
      */
      bool addRuleType(const std::string &ruleTypeName, const std::bitset<optNum> typeSupportOpt);
      bool addRuleType(const std::string &ruleTypeName, const std::string &typeSupportOpt);
      /*
      if this rule's ruleType's typeSupportOpt [2~5] set 1,use int to save value else use string
      */
      std::string addRule(const std::string &ruleTypeName, int ruleUsedOpt, const std::string &compareValue, bool useRule = true); // return rule id
      bool setRule(const std::string &ruleTypeName, std::string &ruleId, int ruleUsedOpt, const std::string &compareValue, bool useRule = true);
      bool findRuleType(const std::string &ruleTypeName);
      bool findRule(const std::string &ruleTypeName, const std::string &ruleId);
      bool delRuleType(const std::string &ruleTypeName);
      bool delRule(const std::string &ruleTypeName, const std::string &ruleId);
      bool matchRule(const std::string &ruleTypeName, const std::string &compareValue);
};
bool Fliter::addRuleType(const std::string &ruleTypeName, const std::bitset<optNum> typeSupportOpt)
{
      if (this->findRuleType(ruleTypeName))
            return false;
      return ruleTypeList.insert(std::pair<const std::string, std::pair<bool, std::shared_ptr<ruleNode>>>(ruleTypeName, std::pair<bool, std::shared_ptr<ruleNode>>((typeSupportOpt[2] || typeSupportOpt[3] || typeSupportOpt[4] || typeSupportOpt[5]), std::make_shared<ruleNode>(ruleNode(typeSupportOpt))))).second;
}
bool Fliter::addRuleType(const std::string &ruleTypeName, const std::string &typeSupportOpt)
{
      if (this->findRuleType(ruleTypeName))
            return false;
      return ruleTypeList.insert(std::pair<const std::string, std::pair<bool, std::shared_ptr<ruleNode>>>(ruleTypeName, std::pair<bool, std::shared_ptr<ruleNode>>((typeSupportOpt[2] || typeSupportOpt[3] || typeSupportOpt[4] || typeSupportOpt[5]), std::make_shared<ruleNode>(ruleNode(std::bitset<Fliter::optNum>(typeSupportOpt)))))).second;
}
std::string Fliter::addRule(const std::string &ruleTypeName, const int ruleUsedOpt, const std::string &compareValue, bool useRule)
{
      if (compareValue.empty())
            return "-3";
      if (!this->findRuleType(ruleTypeName))
            return "-1";
      if (this->findRule(ruleTypeName, std::to_string(ruleUsedOpt) + std::to_string(useRule) + compareValue))
            return "-4";
      auto typeOpt = (*(ruleTypeList[ruleTypeName].second)).UsedOpt;
      if (ruleUsedOpt < 0 && ruleUsedOpt >= optNum && !typeOpt[optNum - ruleUsedOpt])
            return "-2";
      std::shared_ptr<Fliter::ruleNode> lastNode = (ruleTypeList[ruleTypeName].second);
      if (!(ruleTypeList[ruleTypeName].second))
      {
            (ruleTypeList[ruleTypeName].second) = std::make_shared<ruleNode>(ruleUsedOpt);
            lastNode = (ruleTypeList[ruleTypeName].second);
            goto addRule_goto;
      }
      else
      {
            while ((*lastNode).next)
            {
                  lastNode = (*lastNode).next;
            }
            (*lastNode).next = std::make_shared<ruleNode>(ruleUsedOpt);
            lastNode = (*lastNode).next;
            goto addRule_goto;
      }
addRule_goto:
{
      if (ruleTypeList[ruleTypeName].first)
      {
            ruleValue temp(ruleUsedOpt, std::stoi(compareValue));
            (*lastNode).rule = temp;
      }
      else
      {
            ruleValue temp(ruleUsedOpt, compareValue);
            (*lastNode).rule = temp;
      }
}
      (*lastNode).id = std::to_string(ruleUsedOpt) + std::to_string(useRule) + compareValue;
      return (*lastNode).id;
}
bool Fliter::setRule(const std::string &ruleTypeName, std::string &ruleId, const int ruleUsedOpt, const std::string &compareValue, bool useRule)
{
      if (compareValue.empty())
            return false;
      if (!this->findRuleType(ruleTypeName))
            return false;
      if (ruleUsedOpt < 0 && ruleUsedOpt >= optNum && !(ruleTypeList[ruleTypeName].second)->UsedOpt[optNum - ruleUsedOpt])
            return false;
      if (!this->findRule(ruleTypeName, ruleId))
            return false;
      auto thisRule = (ruleTypeList[ruleTypeName].second);
      while ((*thisRule).next)
      {
            if ((*thisRule).id == ruleId)
            {
                  (*thisRule).rule.useRule = useRule;
                  (*thisRule).rule.ruleUsedOpt = ruleUsedOpt;
                  if (ruleTypeList[ruleTypeName].first)
                        (*thisRule).rule.value = std::stoi(compareValue);
                  else
                        (*thisRule).rule.value = compareValue;
                  (*thisRule).id = std::to_string(ruleUsedOpt) + std::to_string(useRule) + compareValue;
                  ruleId = (*thisRule).id;
                  return true;
            }
            thisRule = (*thisRule).next;
      }
      return false;
}
bool Fliter::findRuleType(const std::string &ruleTypeName)
{
      return ruleTypeList.find(ruleTypeName) != ruleTypeList.end();
}
bool Fliter::findRule(const std::string &ruleTypeName, const std::string &ruleId)
{
      if (!this->findRuleType(ruleTypeName))
            return false;
      auto thisRule = (ruleTypeList[ruleTypeName].second);
      while (thisRule && (*thisRule).next)
      {
            if ((*thisRule).id == ruleId)
                  return true;
            thisRule = (*thisRule).next;
      }
      return false;
}
bool Fliter::delRule(const std::string &ruleTypeName, const std::string &ruleId)
{
      if (!this->findRuleType(ruleTypeName) || !this->findRule(ruleTypeName, ruleId) || !(ruleTypeList[ruleTypeName].second))
            return false;
      auto thisRule = (ruleTypeList[ruleTypeName].second);
      auto previousRule = (ruleTypeList[ruleTypeName].second);
      if ((*thisRule).id == ruleId)
      {
            previousRule = (*thisRule).next;
            return true;
      }
      if (!(*thisRule).next)
            thisRule = (*thisRule).next;
      while ((*thisRule).next)
      {
            if ((*thisRule).id == ruleId)
            {
                  (*previousRule).next = (*thisRule).next;
                  return true;
            }
            previousRule = thisRule;
            thisRule = (*thisRule).next;
      }
      return false;
}
bool Fliter::delRuleType(const std::string &ruleTypeName)
{
      if (!this->findRuleType(ruleTypeName))
            return false;
      ruleTypeList.erase(ruleTypeName);
      return true;
}
bool Fliter::matchRule(const std::string &ruleTypeName, const std::string &compareValue)
{
      if (compareValue.empty())
            return false;
      if (!this->findRuleType(ruleTypeName))
            return false;
      if (!(ruleTypeList[ruleTypeName].second))
            return false;
      VariantIntOrString compareValueVariant;
      if (ruleTypeList[ruleTypeName].first)
            compareValueVariant = std::stoi(compareValue);
      else
            compareValueVariant = compareValue;

      for (auto thisRule = (ruleTypeList[ruleTypeName].second); thisRule; thisRule = (*thisRule).next)
      {
            if ((*thisRule).rule.useRule)
            {
                  VariantIntOrString matchValue = (*thisRule).rule.value;
                  switch ((*thisRule).rule.ruleUsedOpt)
                  {
                  case 0: //<
                        return compareValueVariant < matchValue;
                        break;
                  case 1: //<=
                        return compareValueVariant <= matchValue;
                        break;
                  case 2: //>
                        return compareValueVariant > matchValue;
                        break;
                  case 3: //>=
                        return compareValueVariant >= matchValue;
                        break;
                  case 4: //==
                        return compareValueVariant == matchValue;
                        break;
                  case 5: //!=
                        return compareValueVariant != matchValue;
                        break;
                  default:
                        return false;
                  }
            }
      }
      return true;
}
#endif