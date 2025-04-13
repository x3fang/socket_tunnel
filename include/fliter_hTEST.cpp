#include "fliter.h"
#include <iostream>
using std::cout;
using std::endl;
struct datas
{
      std::string wanIp = "127.0.0.1", lanIp = "127.0.0.1", commit = "127.0.0.1";
      int systemKind = 0, use = 0;
};
int main()
{
      datas data;
      Fliter fliter;
      fliter.addRuleType("use", "000011");
      fliter.addRuleType("wanIp", "000011");
      fliter.addRuleType("lanIp", "000011");
      fliter.addRuleType("commit", "000011");
      fliter.addRuleType("systemKind", "000011");
      fliter.addRule("use", 5, "1");
      fliter.addRule("use", 5, "1");
      if ((fliter.matchRule("wanIp", data.wanIp) &&
           fliter.matchRule("lanIp", data.lanIp) &&
           fliter.matchRule("systemKind", std::to_string(data.systemKind)) &&
           fliter.matchRule("commit", data.commit) &&
           fliter.matchRule("use", std::to_string(data.use))))
      {
            cout << "true" << endl;
      }
      return 0;
}