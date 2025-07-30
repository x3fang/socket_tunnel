#include "..\include\globalDefine.h"
#include "..\include\intermediatorStruct.h"
#include "..\include\fliter.h"
#include <iostream>
#include <sstream>
struct talkRoomInfoStruct
{
      std::string sendName;
      std::shared_ptr<std::vector<talkRoomInfoStruct>> recvQueue_client;
      std::vector<std::pair<std::string /*send to who; NULL mean all*/, std::vector<std::string>>> msg;
      bool operator==(const talkRoomInfoStruct &other) const
      {
            return this->sendName == other.sendName;
      }
      bool operator==(const std::string &other) const
      {
            return this->sendName == sendName;
      }
      ~talkRoomInfoStruct()
      {
            recvQueue_client = nullptr;
      }
};
class talkRoomClass
{
public:
      std::shared_ptr<SOCKET> sock;
      std::string selfName;

      std::shared_ptr<std::vector<talkRoomInfoStruct>> recvQueue_client;
      std::shared_ptr<std::vector<talkRoomInfoStruct>> sendQueue_client;
      std::shared_ptr<std::vector<talkRoomInfoStruct>> recvQueue_server;

      std::jthread sendThread;

      bool closeSignal = false;
      bool stopSignal = false, server = false;

      struct talkRoomQueueMapStruct
      {
            std::shared_ptr<std::vector<talkRoomInfoStruct>> recvQueue_server;
      };
      static std::map<std::string /*SEID*/, talkRoomQueueMapStruct> talkRoomQueueMap;

      talkRoomClass()
      {
            recvQueue_client = std::make_shared<std::vector<talkRoomInfoStruct>>();
      }
      ~talkRoomClass() = default;
      void sendMsg_server();
      bool talk();
      void recvMsg_client();
};
void talkRoomClass::sendMsg_server()
{
      while (!closeSignal)
      {
            for (auto &it : *recvQueue_server)
            {
                  if (!it.msg.empty())
                  {
                        if (it.msg.front().first == "NULL")
                        {
                              std::cerr << "send to all" << std::endl;
                              for (auto &it : talkRoomQueueMap)
                              {
                                    std::cout << it.first << std::endl;
                                    for (auto &it2 : *it.second.recvQueue_server)
                                    {
                                          std::cout << it2.sendName << " " << &(*it2.recvQueue_client) << std::endl;
                                    }
                              }
                              for (auto &it : *recvQueue_server)
                              {
                                    std::cout << it.sendName << std::endl;
                              }
                              auto temp = talkRoomInfoStruct();
                              temp.sendName = it.sendName;
                              temp.msg.push_back(it.msg.front());
                              for (auto &client : *recvQueue_server)
                              {
                                    std::cout << "send to " << client.sendName << " " << &(*client.recvQueue_client) << std::endl;
                                    client.recvQueue_client->push_back(temp);
                              }
                        }
                        else
                        {
                              auto recvAddress = std::find(recvQueue_server->begin(), recvQueue_server->end(), it);
                              std::cout << "send to " << it.sendName << std::endl;
                              if (recvAddress != recvQueue_server->end())
                              {
                                    auto temp = talkRoomInfoStruct();
                                    temp.sendName = it.sendName;
                                    temp.msg.push_back(it.msg.front());

                                    recvAddress->recvQueue_client->push_back(temp);
                                    it.msg.erase(it.msg.begin());
                              }
                        }
                        it.msg.erase(it.msg.begin());
                  }
            }
      }

      auto temp = talkRoomInfoStruct();
      temp.sendName = "/server\\";
      temp.msg.push_back(std::pair<std::string, std::vector<std::string>>("/server\\", std::vector<std::string>(1, "exit")));
      for (auto &client : *recvQueue_server)
      {
            client.recvQueue_client->push_back(temp);
      }
      std::cout << "server thread exit" << std::endl;
}
void talkRoomClass::recvMsg_client()
{
      while (!closeSignal)
      {
            std::string receiver, recvBuf;
            std::vector<std::string> data;
            recv(*sock, receiver);
            if (receiver == "/exit")
            {
                  send(*sock, "/exit");
                  closeSignal = true;
                  return;
            }
            while (recvBuf != "end")
            {
                  recv(*sock, recvBuf);
                  if (recvBuf != "end")
                  {
                        data.push_back(recvBuf);
                  }
                  else
                  {
                        break;
                  }
            }
            auto it = std::find(sendQueue_client->begin(), sendQueue_client->end(), selfName);
            if (it != sendQueue_client->end())
            {
                  it->msg.push_back(std::make_pair(receiver, data));
            }
      }
      return;
}
bool talkRoomClass::talk()
{
      std::jthread recv_showThread(&talkRoomClass::recvMsg_client, this);
      while (!closeSignal)
      {
            if (!recvQueue_client->empty())
            {
                  std::cout << selfName << " into  if (!recvQueue_client.empty()) " << &(recvQueue_client) << " " << *sock << std::endl;

                  for (auto &it : *recvQueue_client)
                  {
                        auto &temp = it;
                        send(*sock, temp.sendName);
                        for (auto &it : temp.msg.front().second)
                        {
                              send(*sock, it);
                        }
                        send(*sock, "end");
                  }
                  recvQueue_client->clear();
            }
            else
            {
                  std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
      }
      std::cout << selfName << " exit" << !closeSignal << std::endl;
      auto it = std::find(sendQueue_client->begin(), sendQueue_client->end(), selfName);
      sendQueue_client->erase(it);
      recv_showThread.join();
      return true;
}

class talkRoom : public PluginNamespace::pluginBase
{
public:
      bool runFun(Info &info)
      {
            std::shared_ptr<programPluginInfoStruct> &funInfo = (std::shared_ptr<programPluginInfoStruct> &)(info.customize_data[0]);
            std::shared_ptr<PluginInfoStruct> &pluginInfo = (std::shared_ptr<PluginInfoStruct> &)(info.customize_data[1]);
            std::shared_ptr<talkRoomClass> it = std::make_shared<talkRoomClass>();

            std::string recvBuf;
            std::string sendBuf;

            it->sock = funInfo->mainConnectSocket;

            recv(*it->sock, recvBuf);
            it->selfName = recvBuf;
            recv(*it->sock, recvBuf);
            if (recvBuf == "server")
            {
                  it->server = true;

                  it->recvQueue_server = std::make_shared<std::vector<talkRoomInfoStruct>>();
                  talkRoomClass::talkRoomQueueMap[it->selfName].recvQueue_server = it->recvQueue_server;
                  it->sendThread = std::jthread(&talkRoomClass::sendMsg_server, it);

                  it->sendQueue_client = talkRoomClass::talkRoomQueueMap[it->selfName].recvQueue_server;
            }
            else if (recvBuf == "client")
            {
                  recv(*it->sock, recvBuf); // recv the server's name
                  if (talkRoomClass::talkRoomQueueMap.find(recvBuf) == talkRoomClass::talkRoomQueueMap.end())
                  {
                        std::cout << "Server not found, please check the Name." << std::endl;
                        return false;
                  }
                  it->sendQueue_client = talkRoomClass::talkRoomQueueMap[recvBuf].recvQueue_server;
            }
            auto selfInfo = std::make_shared<talkRoomInfoStruct>();
            selfInfo->sendName = it->selfName;
            selfInfo->recvQueue_client = it->recvQueue_client;

            it->sendQueue_client->push_back(*selfInfo);
            std::cout << "-------" << std::endl;
            std::cout << *it->sock << std::endl;
            std::cout << "3-------" << std::endl;
            for (auto &it : talkRoomClass::talkRoomQueueMap)
            {
                  std::cout << it.first << std::endl;
                  for (auto &it2 : *it.second.recvQueue_server)
                  {
                        std::cout << it2.sendName << " " << &(*it2.recvQueue_client) << std::endl;
                  }
            }
            // for (auto &it : *sendQueue_client)
            // {
            //       std::cout << it.sendName << std::endl;
            // }

            it->talk();

            if (it->server)
            {
                  talkRoomClass::talkRoomQueueMap[funInfo->SEID].recvQueue_server = nullptr;
            }
            return true;
      }
      talkRoom()
      {
            this->used = true;
            this->show = true;
            this->pluginName = "talkRoom";
            this->version = "1.0.0";
            this->author = "x3fang";
      }
};
std::map<std::string /*SEID*/, talkRoomClass::talkRoomQueueMapStruct> talkRoomClass::talkRoomQueueMap;
static talkRoom Plugin_talkRoom;
extern "C"
{
      EXPORT bool registerFun(PluginNamespace::PluginManager &pluginManager)
      {
            return pluginManager.registerFun(&Plugin_talkRoom).first;
      }
}