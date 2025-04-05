#include "..\include\globalDefine.h"
#include "..\include\intermediatorStruct.h"
#include "..\include\fliter.h"
#include <iostream>
#include <sstream>
#include <iomanip>
class find : public PluginNamespace::pluginBase
{
public:
      bool runFun(PluginNamespace::PluginInfo &info) override
      {
            auto sock = info.mainConnectSocket;
            std::string data;
            send(*sock, (*(std::string *)(info.cus->data[1].get())));
            recv(*sock, data);
            if (data == "0") // false
                  return false;
            else
                  return true;
      }
      find()
      {
            this->used = true;
            this->pluginName = "findClient";
            this->version = "1.0.0";
            this->author = "x3fang";
      }
};
class delClient : public PluginNamespace::pluginBase
{
public:
      bool runFun(PluginNamespace::PluginInfo &info) override
      {
            auto sock = info.mainConnectSocket;
            std::string data;
            send(*sock, (*(std::string *)(info.cus->data[1].get())));
            recv(*sock, data);
            if (data == "0") // false
                  return false;
            else
                  return true;
      }
      delClient()
      {
            this->used = true;
            this->pluginName = "delClient";
            this->version = "1.0.0";
            this->author = "x3fang";
      }
};
class showClient : public PluginNamespace::pluginBase
{
public:
      bool runFun(PluginNamespace::PluginInfo &info) override
      {
            system("cls");
            auto sock = info.mainConnectSocket;
            std::string data;
            std::cout << std::setw(9) << std::left << "number"
                      << std::setw(20) << "Wan IP"
                      << std::setw(20) << "Lan IP"
                      << std::setw(15) << "SystemKind"
                      << std::setw(35) << "SEID" << "Commit" << std::endl;
            std::string SEID, wanIp, lanIp, commit, systemKind;
            int outputNum = 0;
            info.cus->data.push_back(std::make_shared<std::vector<std::string>>(10, ""));
            while (data != "end")
            {
                  recv(*sock, data);
                  if (data != "end")
                  {
                        std::stringstream ss(data);
                        ss >> wanIp >> lanIp >> systemKind;
                        SEID = data.substr(data.find(systemKind) + 1, data.find("\r\n"));
                        commit = data.substr(data.find("\r\n") + 2);
                        std::cout << std::setw(9) << std::left << outputNum++
                                  << std::setw(20) << wanIp
                                  << std::setw(20) << lanIp
                                  << std::setw(15) << (systemKind == "0" ? "Windows" : "Linux")
                                  << std::setw(35) << SEID << commit << std::endl;
                        ((std::vector<std::string> *)(info.cus->data[1].get()))->push_back(SEID);
                  }
                  else
                        break;
            }
            std::cout << "Total: " << outputNum << std::endl;
            return true;
      }
      showClient()
      {
            this->used = true;
            this->pluginName = "showClient";
            this->version = "1.0.0";
            this->author = "x3fang";
      }
};
class connectClient : public PluginNamespace::pluginBase
{
public:
      bool runFun(PluginNamespace::PluginInfo &info) override
      {
            auto &pluginInfo = (*((PluginInfoStruct *)(info.cus->data[0].get())));
            auto serverSock = info.mainConnectSocket;
            std::string data;
            while (true)
            {
                  system("cls");
                  pluginInfo.pluginManager->runFun("showClient", info);
                  std::cout << "input a number:";
                  if (kbhit())
                  {
                        int num;
                        std::cin >> num;
                        if (num < 0 || num >= pluginInfo.ClientInfo->size())
                              continue;
                        int res = send(*serverSock, ((std::vector<std::string> *)(info.cus->data[1].get()))->operator[](num));
                        recv(*serverSock, data);
                        if (data == "ok")
                        {
                              recv(*serverSock, data);
                              std::cout << data;
                              while (true)
                              {
                                    std::getline(std::cin, data);
                                    send(*serverSock, data);
                                    recv(*serverSock, data);
                                    if (data == "\r\nend\r\n")
                                    {
                                          send(*serverSock, "\r\nexit\r\n");
                                          break;
                                    }
                                    std::cout << data;
                              }
                        }
                        else
                        {
                              std::cout << "error fail to connect (The client is used or connection disconnect)" << std::endl;
                              getch();
                        }
                  }
                  else
                        Sleep(1500);
            }
            return true;
      }
      connectClient()
      {
            this->used = true;
            this->pluginName = "connectClient";
            this->version = "1.0.0";
            this->author = "x3fang";
      }
};
static find Plugin_find;
static delClient Plugin_delClient;
static showClient Plugin_showClient;
static connectClient Plugin_connectClient;
extern "C"
{
      EXPORT bool registerFun(PluginNamespace::registerFunValue registerFun)
      {
            return registerFun(&Plugin_find).first &&
                   registerFun(&Plugin_delClient).first &&
                   registerFun(&Plugin_showClient).first &&
                   registerFun(&Plugin_connectClient).first;
      }
}