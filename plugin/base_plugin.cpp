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
            auto pluginInfo = std::static_pointer_cast<std::shared_ptr<PluginInfoStruct>>(info.cus->data[0]);
            std::string data;
            Fliter fliter;
            fliter.addRuleType("use", EQUAL | NOT_EQUAL);
            fliter.addRule("use", "0", EQUAL);

            info.cus->data.push_back((std::shared_ptr<void>)(std::make_shared<Fliter>(fliter)));

            while (true)
            {
                  int res = recv(*sock, data);
                  if (res == SUCCESS_STATUS)
                  {
                        if (data == "\r\nexit\r\n")
                              break;
                        std::stringstream ss(data);
                        std::vector<std::string> sendData;
                        sendData.push_back("0");
                        for (auto &it : (*(*pluginInfo)->ClientInfo))
                              if (it.second->lanIp == data)
                                    sendData.push_back(it.second->wanIp + " " +
                                                       it.second->lanIp + " " +
                                                       std::to_string(it.second->systemKind) + " " +
                                                       it.second->SEID + "\r" +
                                                       it.second->commit);
                        while (sendData.back() != "0")
                        {
                              if (send(*sock, sendData.back()) == SUCCESS_STATUS)
                                    sendData.pop_back();
                        }
                        send(*sock, "end");
                  }
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
            if (recv(*sock, data) == SUCCESS_STATUS)
            {
                  std::stringstream ss(data);
                  auto pluginInfo = std::static_pointer_cast<PluginInfoStruct>(info.cus->data[0]);
                  if (send(*sock,
                           (pluginInfo->delClient(data.substr(1))) ? "succeed" : "failed") == SUCCESS_STATUS)
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
            auto pluginInfo = std::static_pointer_cast<PluginInfoStruct>(info.cus->data[0]);
            auto funlog = pluginInfo->log->getFunLog("showClientRunFun");
            funlog->writeln("start");
            auto sock = info.mainConnectSocket;
            std::string sendMsg;
            std::shared_ptr<Fliter> fliter;
            if (info.cus->data.size() >= 2)
            {
                  fliter = std::static_pointer_cast<Fliter>(info.cus->data[1]);
                  fliter->addRuleType("use", EQUAL | NOT_EQUAL);
                  fliter->addRuleType("wanIp", EQUAL | NOT_EQUAL);
                  fliter->addRuleType("lanIp", EQUAL | NOT_EQUAL);
                  fliter->addRuleType("commit", EQUAL | NOT_EQUAL);
                  fliter->addRuleType("systemKind", EQUAL | NOT_EQUAL);
                  funlog->writeln("from 1");
            }
            if (!fliter)
            {
                  fliter = std::make_shared<Fliter>();
                  fliter->addRuleType("use", EQUAL | NOT_EQUAL);
                  fliter->addRuleType("wanIp", EQUAL | NOT_EQUAL);
                  fliter->addRuleType("lanIp", EQUAL | NOT_EQUAL);
                  fliter->addRuleType("commit", EQUAL | NOT_EQUAL);
                  fliter->addRuleType("systemKind", EQUAL | NOT_EQUAL);
                  funlog->writeln("from 2");
            }
            funlog->writeln(std::to_string(pluginInfo->ClientInfo->size()));
            return sendOnce(sock, *(pluginInfo->ClientInfo), *fliter);
      }
      showClient()
      {
            this->used = true;
            this->pluginName = "showClient";
            this->version = "1.0.0";
            this->author = "x3fang";
      }

private:
      bool sendOnce(std::shared_ptr<SOCKET> &sock, std::map<std::string, std::shared_ptr<IndividualInfoStruct>> &infoMap, Fliter &fliter)
      {
            for (auto it = infoMap.begin(); it != infoMap.end(); it++)
            {
                  auto data = it->second.get();
                  if ((fliter.matchRule("wanIp", (*data).wanIp) &&
                       fliter.matchRule("lanIp", (*data).lanIp) &&
                       fliter.matchRule("systemKind", std::to_string((*data).systemKind)) &&
                       fliter.matchRule("commit", (*data).commit) &&
                       fliter.matchRule("use", std::to_string(((*data).use ? 1 : 0)))))
                  {
                        std::string sendMsg((*data).wanIp + " " +
                                            (*data).lanIp + " " +
                                            std::to_string((*data).systemKind) + " " +
                                            (*data).SEID + "\r\n" +
                                            (*data).commit);
                        int res = send(*sock, sendMsg);
                        if (res != SUCCESS_STATUS)
                        {
                              send(*sock, "end");
                              return false;
                        }
                  }
            }
            send(*sock, "end");
            return true;
      }
};
class connectClient : public PluginNamespace::pluginBase
{
public:
      bool runFun(PluginNamespace::PluginInfo &info) override
      {
            bool status = true;
            auto pluginInfo = std::static_pointer_cast<PluginInfoStruct>(info.cus->data[0]);
            auto funlog = pluginInfo->log->getFunLog("connectClientRunFun");
            auto serverSock = info.mainConnectSocket;

            std::string data;

            Fliter fliter;
            fliter.addRuleType("use", EQUAL | NOT_EQUAL);
            fliter.addRule("use", "0", EQUAL);

            info.cus->data.push_back((std::shared_ptr<void>)(std::make_shared<Fliter>(fliter)));

            while (true)
            {
                  pluginInfo->pluginManager->runFun("showClient", info);
                  int res = recv(*serverSock, data);
                  if (res == SUCCESS_STATUS)
                  {
                        if (data == "\r\nnext\r\n")
                              continue;
                        else if (data == "\r\nexit\r\n")
                              break;
                        else if (pluginInfo->ClientInfo->find(data) != pluginInfo->ClientInfo->end())
                        {
                              std::string clientSEID = data;
                              pluginInfo->ClientInfo->at(data)->Lock();
                              auto clientSock = pluginInfo->ClientInfo->at(data)->commSocket;
                              sendPluginList(*pluginInfo->pluginManager, *clientSock);
                              send(*clientSock, pluginName);
                              recv(*clientSock, data);

                              send(*serverSock, data);
                              if (data == "ok")
                              {
                                    recv(*clientSock, data);
                                    send(*serverSock, data);
                                    while (true)
                                    {
                                          data.clear();
                                          recv(*serverSock, data);
                                          send(*clientSock, data);
                                          recv(*clientSock, data);
                                          funlog->writeln(data);
                                          send(*serverSock, data);
                                          if (data == "\r\n[exit]\r\n")
                                                break;
                                    }
                              }
                              data.clear();
                              pluginInfo->ClientInfo->at(clientSEID)->unLock();
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

private:
      void sendPluginList(PluginNamespace::PluginManager &pluginManager, SOCKET &sock)
      {
            auto pluginList = pluginManager.getAllPluginName();
            for (auto &pluginName : pluginList)
                  send(sock, pluginName);
            send(sock, "end");
            return;
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