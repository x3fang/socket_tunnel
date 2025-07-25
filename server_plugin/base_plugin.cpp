#include "..\include\globalDefine.h"
#include "..\include\serverStruct.h"
#include "..\include\fliter.h"
#include <iostream>
#include <sstream>
#include <iomanip>
class find : public PluginNamespace::pluginBase
{
public:
      bool runFun(PluginNamespace::Info &info) override
      {
            if (info.customize_data.empty())
                  return false;

            std::shared_ptr<programPluginInfoStruct> &funInfo = (std::shared_ptr<programPluginInfoStruct> &)(info.customize_data[0]);
            std::shared_ptr<PluginInfoStruct> &pluginInfo = (std::shared_ptr<PluginInfoStruct> &)(info.customize_data[1]);
            std::shared_ptr<SOCKET> sock = funInfo->mainConnectSocket;

            std::string findLanIp, recvData;
            std::string wanIp, LanIp, systemKind, SEID;
            int outputNum = 0;

            while (true)
            {
                  bool intoConnect = false;
                  std::cout << "Input client's  lan IP or input \"exit\" to exit:";
                  std::cin >> findLanIp;
                  if (findLanIp == "exit")
                  {
                        send(*sock, "\r\nexit\r\n");
                        break;
                  }

                  std::cout << std::setw(9) << std::left << "number"
                            << std::setw(20) << "Wan IP"
                            << std::setw(20) << "Lan IP"
                            << std::setw(15) << "SystemKind"
                            << std::setw(35) << "SEID" << "Commit" << std::endl;

                  send(*sock, findLanIp);
                  while (true)
                  {
                        auto res = recv(*sock, recvData);
                        if (res != SUCCESS_STATUS)
                              return false;
                        if (recvData == "end")
                              break;
                        else
                        {
                              std::stringstream ss(recvData);
                              ss >> wanIp >> LanIp >> systemKind >> SEID;
                              std::cout << std::setw(20) << wanIp
                                        << std::setw(20) << LanIp
                                        << std::setw(15) << (systemKind == "0" ? "Windows" : "Linux")
                                        << std::setw(35) << SEID << recvData.substr(recvData.find_last_of('\r')) << std::endl;
                              outputNum++;
                        }
                  }
                  if (outputNum == 0)
                        std::cout << "Not find client.  Cheak your lan IP." << std::endl;
                  if (!intoConnect)
                        Sleep(800);
            }
            getch();
            return true;
      }
      find()
      {
            this->used = true;
            this->show = false;
            this->pluginName = "findClient";
            this->version = "1.0.0";
            this->author = "x3fang";
      }
};
class delClient : public PluginNamespace::pluginBase
{
public:
      bool runFun(PluginNamespace::Info &info) override
      {
            if (info.customize_data.empty() || info.customize_data.size() < 3)
                  return false;

            std::shared_ptr<programPluginInfoStruct> &funInfo = (std::shared_ptr<programPluginInfoStruct> &)(info.customize_data[0]);
            std::shared_ptr<PluginInfoStruct> &pluginInfo = (std::shared_ptr<PluginInfoStruct> &)(info.customize_data[1]);
            std::shared_ptr<SOCKET> sock = funInfo->mainConnectSocket;

            std::string data;
            send(*sock, *(std::shared_ptr<std::string> &)(info.customize_data[2])); // send client SEID
            recv(*sock, data);                                                      // recv status
            if (data == "failed")
                  return false;
            else
                  return true;
      }
      delClient()
      {
            this->used = true;
            this->show = false;
            this->pluginName = "delClient";
            this->version = "1.0.0";
            this->author = "x3fang";
      }
};
class showClient : public PluginNamespace::pluginBase
{
public:
      bool runFun(PluginNamespace::Info &info) override
      {
            if (info.customize_data.empty())
                  return false;

            std::shared_ptr<programPluginInfoStruct> &funInfo = (std::shared_ptr<programPluginInfoStruct> &)(info.customize_data[0]);
            std::shared_ptr<PluginInfoStruct> &pluginInfo = (std::shared_ptr<PluginInfoStruct> &)(info.customize_data[1]);
            std::shared_ptr<SOCKET> sock = funInfo->mainConnectSocket;

            std::string data;
            std::string SEID, wanIp, lanIp, commit, systemKind;
            int outputNum = 0;
            std::shared_ptr<std::vector<std::string>> clientSEIDList = std::make_shared<std::vector<std::string>>();
            // record received client SEID to return

            system("cls");
            std::cout << std::setw(9) << std::left << "number"
                      << std::setw(20) << "Wan IP"
                      << std::setw(20) << "Lan IP"
                      << std::setw(15) << "SystemKind"
                      << std::setw(35) << "SEID" << "Commit" << std::endl;

            while (data != "end")
            {
                  recv(*sock, data);
                  if (data != "end")
                  {
                        std::stringstream ss(data);
                        ss >> wanIp >> lanIp >> systemKind >> SEID;
                        commit = data.substr(data.find("\r\n") + 2);
                        std::cout << std::setw(9) << std::left << outputNum++
                                  << std::setw(20) << wanIp
                                  << std::setw(20) << lanIp
                                  << std::setw(15) << (systemKind == "0" ? "Windows" : "Linux")
                                  << std::setw(35) << SEID << commit << std::endl;
                        (*clientSEIDList).push_back(SEID);
                  }
                  else
                        break;
            }
            info.customize_data.push_back((std::shared_ptr<void>)clientSEIDList);
            info.customize_data.push_back((std::shared_ptr<void>)(std::make_shared<int>(outputNum)));
            std::cout << "Total: " << outputNum << std::endl;
            return true;
      }
      showClient()
      {
            this->used = true;
            this->show = false;
            this->pluginName = "showClient";
            this->version = "1.0.0";
            this->author = "x3fang";
      }
};
class connectClient : public PluginNamespace::pluginBase
{
public:
      bool runFun(PluginNamespace::Info &info) override
      {
            if (info.customize_data.empty())
                  return false;

            std::shared_ptr<programPluginInfoStruct> &funInfo = (std::shared_ptr<programPluginInfoStruct> &)(info.customize_data[0]);
            std::shared_ptr<PluginInfoStruct> &pluginInfo = (std::shared_ptr<PluginInfoStruct> &)(info.customize_data[1]);
            std::shared_ptr<SOCKET> sock = funInfo->mainConnectSocket;
            auto funlog = pluginInfo->log->getFunLog("connectClient");
            std::string data;
            int clientNum = 0;

            while (true)
            {
                  bool intoConnect = false;
                  system("cls");

                  PluginNamespace::Info showClientInfo(info);

                  pluginInfo->pluginManager->runFun("showClient", showClientInfo);

                  clientNum = *std::static_pointer_cast<int>(showClientInfo.customize_data.back());
                  showClientInfo.customize_data.pop_back();

                  std::shared_ptr<std::vector<std::string>> clientSEIDList =
                      std::static_pointer_cast<std::vector<std::string>>(showClientInfo.customize_data.back());
                  showClientInfo.customize_data.pop_back();

                  std::cout << "Input a client number to choose client or input -1 to quit:";
                  if (kbhit())
                  {
                        int num = 0;
                        std::cin >> num;
                        if (num == -1)
                        {
                              send(*sock, "\r\nexit\r\n");
                              break;
                        }
                        if (num < 0 || num >= clientNum)
                        {
                              send(*sock, "\r\nnext\r\n");
                              continue;
                        }
                        int res = send(*sock, (*clientSEIDList)[num]);
                        recv(*sock, data);
                        if (data == "ok")
                        {
                              system("cls");
                              intoConnect = true;
                              recv(*sock, data);
                              std::cout << data;
                              while (true)
                              {
                                    getline(std::cin, data);
                                    if (data.length() == 0)
                                          continue;
                                    send(*sock, data);
                                    data.clear();
                                    recv(*sock, data);
                                    if (data == "\r\n[exit]\r\n")
                                          break;
                                    std::cout << data;
                                    data.clear();
                              }
                        }
                        else
                        {
                              funlog->writeln("Error: fail to connect (The client is used or connection disconnect)");
                        }
                  }
                  if (!intoConnect)
                  {
                        Sleep(800);
                        send(*sock, "\r\nnext\r\n");
                  }
            }
            return true;
      }
      connectClient()
      {
            this->used = true;
            this->show = true;
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
      EXPORT bool registerFun(PluginNamespace::PluginManager &pluginManager)
      {
            return pluginManager.registerFun(&Plugin_find).first &&
                   pluginManager.registerFun(&Plugin_delClient).first &&
                   pluginManager.registerFun(&Plugin_showClient).first &&
                   pluginManager.registerFun(&Plugin_connectClient).first;
      }
}