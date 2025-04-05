#include "..\include\globalDefine.h"
#include "..\include\intermediatorStruct.h"
#include "..\include\fliter.h"
#include <iostream>
#include <sstream>
class find : public PluginNamespace::pluginBase
{
public:
      bool runFun(PluginNamespace::PluginInfo &info) override
      {
            auto sock = info.mainConnectSocket;
            std::string data;
            if (recv(*sock, data) == SUCCESS_OPERAT)
            {
                  std::stringstream ss(data);
                  auto pluginInfo = ((PluginInfoStruct *)(info.cus->data[0].get()));
                  if (send(*sock,
                           (pluginInfo->find(data.substr(1),
                                             (data[0] == 'S'
                                                  ? pluginInfo->ServerInfo.get()
                                                  : pluginInfo->ClientInfo.get()))
                                ? "0"
                                : "1")) == SUCCESS_OPERAT)
                        return true;
                  else
                        return false;
            }
            return false;
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
            if (recv(*sock, data) == SUCCESS_OPERAT)
            {
                  std::stringstream ss(data);
                  auto pluginInfo = ((PluginInfoStruct *)(info.cus->data[0].get()));
                  if (send(*sock,
                           (pluginInfo->delClient(data.substr(1))) ? "0" : "1") == SUCCESS_OPERAT)
                        return true;
                  else
                        return false;
            }
            return false;
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
            auto sock = info.mainConnectSocket;
            std::string sendMsg;
            Fliter fliter;
            if (info.cus->data.size() == 2)
            {
                  fliter = *((Fliter *)(info.cus->data[1].get()));
                  fliter.addRuleType("use", "000011");
                  fliter.addRuleType("wanIp", "000011");
                  fliter.addRuleType("lanIp", "000011");
                  fliter.addRuleType("commit", "000011");
                  fliter.addRuleType("systemKind", "000011");
            }

            for (auto &it : (*((PluginInfoStruct *)info.cus->data[0].get())->ClientInfo.get()))
            {
                  auto data = it.second.get();
                  sendMsg = data->wanIp + " " +
                            data->lanIp + " " +
                            std::to_string(data->systemKind) + " " +
                            data->SEID + "\r\n" +
                            data->commit;
                  if ((fliter.matchRule("wanIp", data->wanIp) && fliter.matchRule("lanIp", data->lanIp) &&
                       fliter.matchRule("systemKind", std::to_string(data->systemKind)) && fliter.matchRule("commit", data->commit) &&
                       fliter.matchRule("use", std::to_string(data->use))))
                        if (send(*sock, sendMsg) != SUCCESS_OPERAT)
                              return false;
            }
            send(*sock, "end");
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
            auto pluginInfo = (*((PluginInfoStruct *)info.cus->data[0].get()));
            auto serverSock = info.mainConnectSocket;
            std::string data;
            Fliter fliter;
            fliter.addRuleType("use", "000011");
            fliter.addRule("use", 0, "1");
            info.cus->data.push_back(std::make_shared<Fliter>(fliter));
            while (true)
            {
                  pluginInfo.pluginManager->runFun("showClient", info);
                  int res = recv(*serverSock, data);
                  if (res == SUCCESS_OPERAT)
                  {
                        if (data == "\r\nnext\r\n")
                              continue;
                        else if (data == "\r\nexit\r\n")
                              break;
                        else if (pluginInfo.ClientInfo->find(data) != pluginInfo.ClientInfo->end())
                        {
                              std::cout << "1";
                              pluginInfo.ClientInfo->at(data)->use = true;
                              auto clientSock = pluginInfo.ClientInfo->at(data)->commSocket;
                              send(*serverSock, "ok");
                              send(clientSock, "connect");
                              recv(clientSock, data);
                              send(*serverSock, data);
                              while (true)
                              {
                                    if (recv(*serverSock, data) == SUCCESS_OPERAT)
                                    {
                                          if (data == "\r\nexit\r\n")
                                                break;
                                    }
                                    send(clientSock, data);
                                    recv(clientSock, data);
                                    send(*serverSock, data);
                              }
                              pluginInfo.ClientInfo->at(data)->use = false;
                              data.clear();
                        }
                        else
                              send(*serverSock, "failed");
                  }
                  else
                        return false;
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