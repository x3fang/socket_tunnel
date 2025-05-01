#include "fliter.h"
#include <iostream>
using std::cout;
using std::endl;
struct datas
{
      std::string wanIp = "127.0.0.1", lanIp = "127.0.0.1", commit = "127.0.0.1";
      int systemKind = 0, use = 1;
};
int main()
{
      // cout << (NOT_EQUAL).to_string();
      datas data;
      Fliter fliter;
      fliter.addRuleType("use", EQUAL | NOT_EQUAL);
      fliter.addRuleType("wanIp", EQUAL | NOT_EQUAL);
      fliter.addRuleType("lanIp", EQUAL | NOT_EQUAL);
      fliter.addRuleType("commit", EQUAL | NOT_EQUAL);
      fliter.addRuleType("systemKind", EQUAL | NOT_EQUAL);
      fliter.addRule("use", "1", EQUAL);
      fliter.addRule("wanIp", "127.0.0.1", EQUAL);
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