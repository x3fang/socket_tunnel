#ifndef FLITER_H
#define FLITER_H
#include <map>
#include <cstring>
#include <bitset>
#include <variant>
#include <memory>
/*< 000001
<= 000010
> 000100
>= 001000
== 010000
!= 100000*/
// 0 1 2 3 4 5
#define LESSTHAN std::bitset<optNum>(32)          //<
#define LESSTHAN_OR_EQUAL std::bitset<optNum>(16) //<=
#define MORETHAN std::bitset<optNum>(8)           //>
#define MORETHAN_OR_EQUAL std::bitset<optNum>(4)  //>=
#define EQUAL std::bitset<optNum>(2)              //==
#define NOT_EQUAL std::bitset<optNum>(1)          //!=

const int optNum = 6;

std::map<std::string, std::bitset<optNum>> FliterOption = {
	{"LESSTHAN", LESSTHAN},                   //<
	{"LESSTHAN_OR_EQUAL", LESSTHAN_OR_EQUAL}, //<=
	{"MORETHAN", MORETHAN},                   //>
	{"MORETHAN_OR_EQUAL", MORETHAN_OR_EQUAL}, //>=
	{"EQUAL", EQUAL},                         //==
	{"NOT_EQUAL", NOT_EQUAL}                  //!=
};

class Fliter
{
private:
	using VariantIntOrString = std::variant<int, std::string>;
	struct ruleValue
	{
		bool useRule = true;
		VariantIntOrString value;
		int ruleUsedOpt = 0;
		ruleValue() { ruleUsedOpt = 0; }
		ruleValue(const int ruleUsedOpt_) : ruleUsedOpt(ruleUsedOpt_) {}
		ruleValue(const int ruleUsedOpt_, VariantIntOrString value_) : ruleUsedOpt(ruleUsedOpt_), value(std::move(value_)) {}
		ruleValue& operator=(ruleValue& other)
		{
			this->ruleUsedOpt = other.ruleUsedOpt;
			this->value = other.value;
			this->useRule = other.useRule;
			return *this;
		}
	};
	struct ruleNode
	{
		std::string id;
		std::bitset<optNum> UsedOpt;
		ruleValue rule;
		ruleNode(const std::bitset<optNum> UsedOpt_) : next(nullptr), UsedOpt(UsedOpt_) {}
		std::shared_ptr<ruleNode> next;
	};
	struct ruleTypeNode
	{
		bool valueInt = true;
		std::bitset<optNum> operatorSet;
		std::shared_ptr<ruleNode> ruleNodes = nullptr;
		ruleTypeNode() = default;
		ruleTypeNode(const std::string& UsedOpt_) : operatorSet(std::bitset<optNum>(UsedOpt_)) {}
		ruleTypeNode(const std::bitset<optNum>& usedOpt) : operatorSet(usedOpt) {}
		ruleTypeNode(bool valueIsInt, const std::bitset<optNum>& usedOpt) : valueInt(valueIsInt), operatorSet(usedOpt) {}
	};
	//< <= > >= == !=
	// 0 1 2 3 4 5
	std::map<std::string, ruleTypeNode> ruleTypeList;

public:
	Fliter() = default;
	Fliter(void*);
	/*
	if this ruleType's typeSupportOpt [2~5] set 1,use int to save value else use string
	*/
	bool addRuleType(const std::string& ruleTypeName, const std::bitset<optNum> typeSupportOpt);
	bool addRuleType(const std::string& ruleTypeName, const std::string& typeSupportOpt);
	std::vector<std::string> getAllRuleType();
	/*
	if this rule's ruleType's typeSupportOpt [2~5] set 1,use int to save value else use string
	*/
	std::string addRule(const std::string& ruleTypeName, const std::string& compareValue, const int ruleUsedOpt, bool useRule = true);
	std::string addRule(const std::string& ruleTypeName, const std::string& compareValue, const std::bitset<optNum> ruleUsedOpt, bool useRule = true);
	bool setRule(const std::string& ruleTypeName, std::string& ruleId, int ruleUsedOpt, const std::string& compareValue, bool useRule = true);
	bool findRuleType(const std::string& ruleTypeName);
	bool findRule(const std::string& ruleTypeName, const std::string& ruleId);
	bool delRuleType(const std::string& ruleTypeName);
	bool delRule(const std::string& ruleTypeName, const std::string& ruleId);
	bool matchRule(const std::string& ruleTypeName, const std::string& compareValue);
};
Fliter::Fliter(void* p)
{
	ruleTypeList = (*(Fliter*)p).ruleTypeList;
}
bool Fliter::addRuleType(const std::string& ruleTypeName, const std::bitset<optNum> typeSupportOpt)
{
	if (this->findRuleType(ruleTypeName) || ruleTypeName.empty())
		return false;
	ruleTypeNode temp(typeSupportOpt[0] || typeSupportOpt[1] || typeSupportOpt[2] || typeSupportOpt[3],
		typeSupportOpt);
	return ruleTypeList.insert(std::pair<std::string, ruleTypeNode>(ruleTypeName, temp)).second;
}
bool Fliter::addRuleType(const std::string& ruleTypeName, const std::string& typeSupportOpt)
{
	if (this->findRuleType(ruleTypeName) || ruleTypeName.empty())
		return false;
	ruleTypeNode temp(typeSupportOpt[0] == '1' || typeSupportOpt[1] == '1' || typeSupportOpt[2] == '1' || typeSupportOpt[3] == '1',
		std::bitset<optNum>(typeSupportOpt));
	return ruleTypeList.insert(std::pair<std::string, ruleTypeNode>(ruleTypeName, temp)).second;
}
std::vector<std::string> Fliter::getAllRuleType()
{
	std::vector<std::string> temp;
	for (auto i : ruleTypeList)
		temp.push_back(i.first);
	return temp;
}
std::string Fliter::addRule(const std::string& ruleTypeName, const std::string& compareValue, const std::bitset<optNum> ruleUsedOpt, bool useRule)
{
	int ruleUsedOptInt = static_cast<int>(ruleUsedOpt.to_string().find_first_of('1'));
	return this->addRule(ruleTypeName, compareValue, ruleUsedOptInt, useRule);
}
std::string Fliter::addRule(const std::string& ruleTypeName, const std::string& compareValue, const int ruleUsedOpt, bool useRule)
{
	if (compareValue.empty())
		return "-3";
	if (!this->findRuleType(ruleTypeName))
		return "-1";
	if (this->findRule(ruleTypeName, std::to_string(ruleUsedOpt) + std::to_string(useRule) + compareValue))
		return "-4";
	auto typeOpt = ruleTypeList[ruleTypeName].operatorSet;
	if (ruleUsedOpt < 0 && ruleUsedOpt >= optNum && !typeOpt[optNum - ruleUsedOpt])
		return "-2";
	std::shared_ptr<Fliter::ruleNode> lastNode = (ruleTypeList[ruleTypeName].ruleNodes);
	if (!(ruleTypeList[ruleTypeName].ruleNodes))
	{
		(ruleTypeList[ruleTypeName].ruleNodes) = std::make_shared<ruleNode>(ruleUsedOpt);
		lastNode = ruleTypeList[ruleTypeName].ruleNodes;
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
		if (ruleTypeList[ruleTypeName].valueInt)
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
bool Fliter::setRule(const std::string& ruleTypeName, std::string& ruleId, const int ruleUsedOpt, const std::string& compareValue, bool useRule)
{
	if (compareValue.empty())
		return false;
	if (!this->findRuleType(ruleTypeName))
		return false;
	if (ruleUsedOpt < 0 && ruleUsedOpt >= optNum && !(ruleTypeList[ruleTypeName].ruleNodes)->UsedOpt[optNum - ruleUsedOpt])
		return false;
	if (!this->findRule(ruleTypeName, ruleId))
		return false;
	auto thisRule = (ruleTypeList[ruleTypeName].ruleNodes);
	while ((*thisRule).next)
	{
		if ((*thisRule).id == ruleId)
		{
			(*thisRule).rule.useRule = useRule;
			(*thisRule).rule.ruleUsedOpt = ruleUsedOpt;
			if (ruleTypeList[ruleTypeName].valueInt)
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
bool Fliter::findRuleType(const std::string& ruleTypeName)
{
	return ruleTypeList.find(ruleTypeName) != ruleTypeList.end();
}
bool Fliter::findRule(const std::string& ruleTypeName, const std::string& ruleId)
{
	if (!this->findRuleType(ruleTypeName))
		return false;
	auto thisRule = (ruleTypeList[ruleTypeName].ruleNodes);
	while (thisRule)
	{
		if ((*thisRule).id == ruleId)
			return true;
		thisRule = (*thisRule).next;
	}
	return false;
}
bool Fliter::delRule(const std::string& ruleTypeName, const std::string& ruleId)
{
	if (!this->findRuleType(ruleTypeName) || !this->findRule(ruleTypeName, ruleId) || !(ruleTypeList[ruleTypeName].ruleNodes))
		return false;
	auto thisRule = (ruleTypeList[ruleTypeName].ruleNodes);
	auto previousRule = (ruleTypeList[ruleTypeName].ruleNodes);
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
bool Fliter::delRuleType(const std::string& ruleTypeName)
{
	if (!this->findRuleType(ruleTypeName))
		return false;
	ruleTypeList.erase(ruleTypeName);
	return true;
}
bool Fliter::matchRule(const std::string& ruleTypeName, const std::string& compareValue)
{
	if (compareValue.empty())
		return false;
	if (!this->findRuleType(ruleTypeName))
		return false;
	if (!(ruleTypeList[ruleTypeName].ruleNodes))
		return true;
	VariantIntOrString compareValueVariant;
	if (ruleTypeList[ruleTypeName].valueInt)
		compareValueVariant = std::stoi(compareValue);
	else
		compareValueVariant = compareValue;

	for (auto thisRule = (ruleTypeList[ruleTypeName].ruleNodes); thisRule; thisRule = (*thisRule).next)
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