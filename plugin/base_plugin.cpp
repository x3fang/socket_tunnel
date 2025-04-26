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
                  auto pluginInfo = (PluginInfoStruct *)(info.cus->data[0]);
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
                  auto pluginInfo = (PluginInfoStruct *)(info.cus->data[0]);
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
            auto pluginInfo = (PluginInfoStruct *)(info.cus->data[0]);
            auto funlog = pluginInfo->log->getFunLog("showClientRunFun");
            auto sock = info.mainConnectSocket;
            std::string sendMsg;
            Fliter *fliter;
            if (info.cus->data.size() == 2)
            {
                  fliter = (Fliter *)(info.cus->data[1]);
                  if (!fliter)
                        fliter = new Fliter();
                  fliter->addRuleType("use", EQUAL | NOT_EQUAL);
                  fliter->addRuleType("wanIp", EQUAL | NOT_EQUAL);
                  fliter->addRuleType("lanIp", EQUAL | NOT_EQUAL);
                  fliter->addRuleType("commit", EQUAL | NOT_EQUAL);
                  fliter->addRuleType("systemKind", EQUAL | NOT_EQUAL);
            }
            funlog->writeln(std::to_string(pluginInfo->ClientInfo->size()));
            std::cout << &pluginInfo->ClientInfo << std::endl;
            system("pause");
            for (auto it = (pluginInfo->ClientInfo)->begin();
                 it != (pluginInfo->ClientInfo)->end(); it++)
            {
                  auto data = it->second.get();

                  // if ((fliter->matchRule("wanIp", data->wanIp) &&
                  //      fliter->matchRule("lanIp", data->lanIp) &&
                  //      fliter->matchRule("systemKind", std::to_string(data->systemKind)) &&
                  //      fliter->matchRule("commit", data->commit) &&
                  //      fliter->matchRule("use", std::to_string((data->use ? 1 : 0)))))
                  // {
                  sendMsg = data->wanIp + " " +
                            data->lanIp + " " +
                            std::to_string(data->systemKind) + " " +
                            data->SEID + "\r\n" +
                            data->commit;
                  int res = send(*sock, sendMsg);
                  if (res != SUCCESS_OPERAT)
                  {
                        funlog->writeln(("send error:" + std::to_string(res)));
                        send(*sock, "end");
                        delete fliter;
                        return false;
                  }
                  // }
            }
            send(*sock, "end");
            delete fliter;
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
            bool status = true;

            auto pluginInfo = (PluginInfoStruct *)(info.cus->data[0]);
            auto funlog = pluginInfo->log->getFunLog("connectClientRunFun");
            auto serverSock = info.mainConnectSocket;

            std::string data;

            Fliter fliter;
            fliter.addRuleType("use", EQUAL | NOT_EQUAL);
            fliter.addRule("use", "0", EQUAL);

            info.cus->data.push_back((void *)(&fliter));

            while (true)
            {
                  pluginInfo->pluginManager->runFun("showClient", info);
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
                              send(clientSock, pluginName);
                              recv(clientSock, data);
                              send(*serverSock, data);
                              if (data == "ok")
                              {
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
                              }
                              pluginInfo->ClientInfo->at(data)->use = false;
                              data.clear();
                        }
                        else
                              send(*serverSock, "failed");
                  }
                  else
                  {
                        status = false;
                        break;
                  }
            }
            return status;
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