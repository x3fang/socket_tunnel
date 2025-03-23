#include "httplib.h"
#include <string>
#include <iostream>
#include <time.h>
#include <vector>
inline const std::string getMyWanIp(void)
{
      static httplib::Client cli("http://ipinfo.io");
      const auto &res = cli.Get("/json");
      if (res && res->status == 200)
      {
            const std::string &keyword("\"ip\": \"");
            const std::string &ret = res->body.substr(res->body.find(keyword) + keyword.length());
            return ret.substr(0, ret.find("\""));
      }
      return "NULL";
}
std::string getMyLanIp()
{
      char name[256];

      int getNameRet = gethostname(name, sizeof(name));

      hostent *host = gethostbyname(name);

      if (!host)
            return "NULL";
      in_addr *pAddr = (in_addr *)*host->h_addr_list;
      for (int i = 0; i < (strlen((char *)*host->h_addr_list) - strlen(host->h_name)) / 4 && pAddr; i++)
      {
            std::string addr = inet_ntoa(pAddr[i]);
            std::cout << addr << std::endl;
            // if (addr != "127.0.0.1")
            // return addr;
      }
      return "127.0.0.1";
}
int main()
{
      // std::cout << getMyLanIp() << std::endl;
      std::vector<std::string> a(20);
      a[1] = "1";
      std::cout << a[1] << std::endl;
      std::cout << *(a.begin() + 1) << std::endl;
      // const double re = 5;
      // int i = 0;
      // clock_t start, end;
      // start = clock();
      // for (i = 0; i < re; i++)
      //       getMyWanIp();
      // end = clock();
      // std::cout << ((double)end - start / CLK_TCK) / re << std::endl;
      getchar();
}