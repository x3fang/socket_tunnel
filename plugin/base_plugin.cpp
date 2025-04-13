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
                  auto pluginInfo = std::static_pointer_cast<PluginInfoStruct>(info.cus->data[0]);
                  std::vector<std::string> sendData;
                  sendData.push_back("0");
                  if (data[0] == 'S')
                  {
                        for (auto &it : (*pluginInfo->ServerInfo))
                              if (it.second->lanIp == data.substr(1))
                                    sendData.push_back(it.second->wanIp + " " +
                                                       it.second->lanIp + " " +
                                                       std::to_string(it.second->systemKind) + " " +
                                                       it.second->SEID);
                  }
                  else if (data[0] == 'C')
                  {
                        for (auto &it : (*pluginInfo->ClientInfo))
                              if (it.second->lanIp == data.substr(1))
                                    sendData.push_back(it.second->wanIp + " " +
                                                       it.second->lanIp + " " +
                                                       std::to_string(it.second->systemKind) + " " +
                                                       it.second->SEID);
                  }
                  while (sendData.back() != "0")
                  {
                        if (send(*sock, sendData.back()) == SUCCESS_OPERAT)
                              sendData.pop_back();
                        else
                              return false;
                  }
                  send(*sock, "end");
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
                  auto pluginInfo = std::static_pointer_cast<PluginInfoStruct>(info.cus->data[0]);
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
            std::shared_ptr<Fliter> fliter;
            if (info.cus->data.size() == 2)
            {
                  fliter = std::static_pointer_cast<Fliter>(info.cus->data[1]);
                  fliter->addRuleType("use", "000011");
                  fliter->addRuleType("wanIp", "000011");
                  fliter->addRuleType("lanIp", "000011");
                  fliter->addRuleType("commit", "000011");
                  fliter->addRuleType("systemKind", "000011");
            }

            for (auto &it : (*std::static_pointer_cast<PluginInfoStruct>(info.cus->data[0])->ClientInfo.get()))
            {
                  auto data = it.second.get();

                  if ((fliter->matchRule("wanIp", data->wanIp) &&
                       fliter->matchRule("lanIp", data->lanIp) &&
                       fliter->matchRule("systemKind", std::to_string(data->systemKind)) &&
                       fliter->matchRule("commit", data->commit) &&
                       fliter->matchRule("use", std::to_string(data->use))))
                  {
                        sendMsg = data->wanIp + " " +
                                  data->lanIp + " " +
                                  std::to_string(data->systemKind) + " " +
                                  data->SEID + "\r\n" +
                                  data->commit;
                        if (send(*sock, sendMsg) != SUCCESS_OPERAT)
                              return false;
                  }
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
            auto pluginInfo = std::static_pointer_cast<PluginInfoStruct>(info.cus->data[0]);
            auto serverSock = info.mainConnectSocket;
            std::string data;
            Fliter fliter;
            fliter.addRuleType("use", "000011");
            fliter.addRule("use", 0, "1");
            info.cus->data.push_back(std::make_shared<Fliter>(fliter));
            while (true)
            {
                  pluginInfo->pluginManager->runFun("showClient", info);
                  std::cout << 1 << std::endl;
                  int res = recv(*serverSock, data);
                  if (res == SUCCESS_OPERAT)
                  {
                        if (data == "\r\nnext\r\n")
                              continue;
                        else if (data == "\r\nexit\r\n")
                              break;
                        else if (pluginInfo->ClientInfo->find(data) != pluginInfo->ClientInfo->end())
                        {
                              pluginInfo->ClientInfo->at(data)->use = true;
                              auto clientSock = pluginInfo->ClientInfo->at(data)->commSocket;
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
                              pluginInfo->ClientInfo->at(data)->use = false;
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
      EXPORT bool registerFun(PluginNamespace::PluginManager &pluginManager)
      {
            return pluginManager.registerFun(&Plugin_find).first &&
                   pluginManager.registerFun(&Plugin_delClient).first &&
                   pluginManager.registerFun(&Plugin_showClient).first &&
                   pluginManager.registerFun(&Plugin_connectClient).first;
      }
}