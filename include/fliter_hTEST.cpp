#include "fliter.h"
#include <iostream>
using std::cout;
using std::endl;
int main()
{
      Fliter fliter;
      cout << fliter.addRuleType("test", std::bitset<Fliter::optNum>("11111"));
      cout << fliter.addRule("test", 0, "123");
      cout << fliter.addRule("test", 0, "123");
      cout << fliter.addRule("test", 4, "123");
      cout << fliter.matchRule("test", "123");
      return 0;
}