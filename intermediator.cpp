#include "include\globalDefine.h"
#include "include\log.h"
#include "include\MD5.h"
#include "include\intermediatorStruct.h"
#include <random>
bool ServerStopFlag = false;
WSADATA g_wsaData;
sockaddr_in g_sockaddr;
std::thread healthyBeatThread;
std::shared_ptr<std::map<std::string, std::shared_ptr<IndividualInfoStruct>>> ServerInfo(std::make_shared<std::map<std::string, std::shared_ptr<IndividualInfoStruct>>>());
std::shared_ptr<std::map<std::string, std::shared_ptr<IndividualInfoStruct>>> ClientInfo(std::make_shared<std::map<std::string, std::shared_ptr<IndividualInfoStruct>>>());
std::vector<healthyBeatInfoStruct> healthyBeatSOCKETList;
std::vector<std::thread> serverThreadArry;

int initServer(SOCKET &ListenSocket, WSADATA &wsaData, sockaddr_in &sockAddr, int port)

{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
      int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
      if (iResult != 0)
      {
            return iResult;
      }
#endif
      // 创建套接字
      ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
      if (ListenSocket == INVALID_SOCKET)
      {
            int errorCode = WSAGetLastError();
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
            WSACleanup();
#endif
            return errorCode;
      }
      // 绑定套接字

      sockAddr.sin_family = AF_INET;
      sockAddr.sin_addr.s_addr = INADDR_ANY;
      sockAddr.sin_port = htons(port);
      if (bind(ListenSocket, (SOCKADDR *)&sockAddr, sizeof(sockAddr)) == SOCKET_ERROR)
      {
            int errorCode = WSAGetLastError();
            closesocket(ListenSocket);
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
            WSACleanup();
#endif
            return errorCode;
      }
      // 监听套接字
      if (listen(ListenSocket, 5) == SOCKET_ERROR)
      {
            closesocket(ListenSocket);
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
            WSACleanup();
#endif
            int errorCode = WSAGetLastError();
            return errorCode;
      }
      return 0;
}
const std::string createSEID(const std::string &temp)
{
      MD5::MD5 md5;
      return md5.encode(temp);
}
bool registerCOS(SOCKET socket,
                 const std::string &wanIp,
                 const std::string &lanIp,
                 int systemKind,
                 const std::string &commit,
                 std::string &SEID_res,
                 std::map<std::string, std::shared_ptr<IndividualInfoStruct>> &infoMap)
{
      static auto prlog = (*g_log).getFunLog("registerCOS");
      std::string SEID = createSEID(lanIp + wanIp + std::to_string(systemKind) + commit);
      prlog->writeln("SEID:" + SEID);
      if (infoMap.find(SEID) == infoMap.end())
      {
            infoMap[SEID] = std::make_shared<IndividualInfoStruct>(SEID, wanIp, lanIp, systemKind, commit, socket, INVALID_SOCKET);
            SEID_res = SEID;
            return true;
      }
      return false;
}
inline bool registerClient(SOCKET socket, const std::string &wanIp, const std::string &lanIp, int systemKind, const std::string &commit, std::string &SEID_res)
{
      return registerCOS(socket, wanIp, lanIp, systemKind, commit, SEID_res, *ClientInfo);
}
inline bool registerServer(SOCKET socket, const std::string &wanIp, const std::string &lanIp, int systemKind, const std::string &commit, std::string &SEID_res)
{
      return registerCOS(socket, wanIp, lanIp, systemKind, commit, SEID_res, *ServerInfo);
}
bool del(const std::string &SEID, std::map<std::string, std::shared_ptr<IndividualInfoStruct>> *infoMap)
{
      if ((*infoMap).find(SEID) != (*infoMap).end())
      {
            (*(*infoMap)[SEID]).Lock();
            (*(*infoMap)[SEID]).del = true;
            if (*(*infoMap)[SEID]->healthSocket != INVALID_SOCKET)
            {
                  send(*(*infoMap)[SEID]->healthSocket, "del");
                  closesocket(*(*infoMap)[SEID]->healthSocket);
                  *(*infoMap)[SEID]->healthSocket = INVALID_SOCKET;
                  healthyBeatSOCKETList.erase(std::find(healthyBeatSOCKETList.begin(), healthyBeatSOCKETList.end(), SEID));
            }
            closesocket(*(*infoMap)[SEID]->commSocket);
            *(*infoMap)[SEID]->commSocket = INVALID_SOCKET;
            (*(*infoMap)[SEID]).unLock();
            (*infoMap).erase(SEID);
            return true;
      }
      return false;
}
inline bool find(const std::string &SEID, std::shared_ptr<std::map<std::string, std::shared_ptr<IndividualInfoStruct>>> infoMap)
{
      return (*infoMap).find(SEID) != (*infoMap).end();
}
bool delClient(const std::string &SEID)
{
      static auto prlog = (*g_log).getFunLog("delClient");
      prlog->writeln("Del client's SEID:" + SEID);
      return del(SEID, &*ClientInfo);
}
bool delServer(const std::string &SEID)
{
      static auto prlog = (*g_log).getFunLog("delServer");
      prlog->writeln("Del server's SEID:" + SEID);
      return del(SEID, &*ServerInfo);
}
bool arrangeRegister(const std::string &buf, std::string &lanIp_res, int &systemKind_res, std::string &commit_res)
{
      static auto prlog = (*g_log).getFunLog("arrangeRegister");
      prlog->writeln("buf:" + buf);
      int LSS = buf.find_first_not_of("0123456789.", 1); // lanIp and system kinds separator
      std::string lanIP = buf.substr(1, LSS - 1);
      if (!lanIP.empty() &&
          std::count(lanIP.begin(), lanIP.end(), '.') == 3 &&
          inet_addr(lanIP.c_str()) != INADDR_NONE)
      {
            lanIp_res = lanIP;
            std::string commit;
            int systemKind = -1;
            if (LSS != std::string::npos)
                  systemKind = (buf[LSS] == 'W' ? 0 : 1);
            commit = buf.substr(LSS + 1);
            commit_res = commit;
            systemKind_res = systemKind;
            return true;
      }
      return false;
}
void sendPluginList(PluginNamespace::PluginManager &pluginManager, SOCKET &sock)
{
      auto pluginList = pluginManager.getAllPluginName();
      for (auto &pluginName : pluginList)
            send(sock, pluginName);
      send(sock, "end");
      return;
}
void healthyBeat()
{
      while (DEBUG)
            ;
      static auto prlog = (*g_log).getFunLog("healthyBeat");
      std::default_random_engine e;
      e.seed(time(0));
      std::string buf;
      while (!ServerStopFlag)
      {
            if (!healthyBeatSOCKETList.empty())
            {
                  auto radom = e();
                  for (auto &healthyInfo : healthyBeatSOCKETList)
                  {
                        int timeout = 3000;
                        setsockopt(healthyInfo.healthSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
                        setsockopt(healthyInfo.healthSocket, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout));
                        send(healthyInfo.healthSocket, std::to_string(radom));
                        recv(healthyInfo.healthSocket, buf);
                        if (buf != std::to_string(radom))
                        {
                              prlog->writeln(std::string("one ") + std::string((healthyInfo.server ? "server" : "client")) +
                                             " disconnect\n" + "SEID:" +
                                             healthyInfo.SEID);
                        }
                        buf.clear();
                  }
            }
            Sleep(500);
      }
}
void serverThread(const std::string SEID)
{
      auto prlog = (*g_log).getFunLog("ST" + SEID);
      auto info = (*ServerInfo)[SEID];
      prlog->writeln("waiting for server healthy socket connect");
      while (*info->healthSocket == INVALID_SOCKET)
            ;
      prlog->writeln("server healthy socket connect success");
      info->Lock();
      std::string buf;
      while (!ServerStopFlag)
      {
            sendPluginList(*pluginManager, *info->commSocket);
            int res = recv(*info->commSocket, buf);
            if (res == SUCCESS_OPERAT)
            {
                  if (buf.find("\r\nexit\r\n") != std::string::npos)
                  {
                        prlog->writeln("server exit");
                        info->unLock();
                        delServer(SEID);
                        return;
                  }
                  else
                  {
                        prlog->writeln("plugin name:" + buf);
                        std::shared_ptr<PluginInfoStruct> PInfo(new PluginInfoStruct);
                        (*PInfo).ClientInfo = ::ClientInfo;
                        (*PInfo).ServerInfo = ::ServerInfo;
                        (*PInfo).find = ::find;
                        (*PInfo).delClient = ::delClient;
                        PluginInfo runFunInfo;
                        runFunInfo.cus = std::make_shared<pluginInfo>();
                        runFunInfo.cus->data.push_back((std::shared_ptr<void>)PInfo);
                        runFunInfo.mainConnectSocket = info->commSocket;
                        runFunInfo.healthySocket = info->healthSocket;
                        if ((*pluginManager).findFun(buf))
                        {
                              send(*info->commSocket, "ok");
                              if ((*pluginManager).runFun(buf, runFunInfo))
                                    send(*info->commSocket, "ok");
                              else
                                    send(*info->commSocket, "failed");
                              continue;
                        }
                        send(*info->commSocket, "failed");
                        prlog->writeln("runFun failed");
                  }
            }
            else
            {
                  prlog->writeln("recv error:" + std::to_string(res) + " at:" + std::to_string(__LINE__));
                  send(*info->commSocket, "failed");
            }
      }
      info->unLock();
      return;
}
int main()
{
      (*connectIp) = "0.0.0.0";
      connectPort = 6020;
      (*g_log).setName("intermediator");
      (*g_log) << "program start" << endl;
      PluginNamespace::loadPlugin((*pluginManager), ".\\plugin\\");
      int res = initServer(*mainConnectSocket, g_wsaData, g_sockaddr, connectPort);
      healthyBeatThread = std::thread(healthyBeat);
      while (true)
      {
            std::string buf;
            SOCKET aptSocket;
            sockaddr_in aptsocketAddr = {0};
            int len = sizeof(aptsocketAddr);
            aptSocket = accept(*mainConnectSocket, (SOCKADDR *)&aptsocketAddr, &len);
            (*g_log).writeln("accept a connect");
            if (aptSocket != INVALID_SOCKET)
            {
                  recv(aptSocket, buf);
                  if (buf.length() <= 1)
                  {
                        closesocket(aptSocket);
                        continue;
                  }
                  std::string wanIp = inet_ntoa(aptsocketAddr.sin_addr), lanIp, commit, SEID;
                  int systemKind;
                  switch (buf[0])
                  {
                  case 'C': // Client
                        if (arrangeRegister(buf, lanIp, systemKind, commit))
                        {
                              if (registerClient(aptSocket, wanIp, lanIp, systemKind, commit, SEID))
                              {
                                    (*g_log).writeln("Register client,SEID:" + SEID +
                                                     "\nwan ip:" + wanIp +
                                                     "\nlan ip:" + lanIp +
                                                     "\nsystemKind:" + (systemKind == 0 ? "Windows" : "Linux") +
                                                     "\ncommit:" + commit);
                                    send(aptSocket, "OK");
                                    send(aptSocket, SEID);
                              }
                              else
                                    goto failRegister;
                        }
                        else
                              goto failRegister;
                        break;
                  case 'S': // Server
                        if (arrangeRegister(buf, lanIp, systemKind, commit))
                        {
                              if (registerServer(aptSocket, wanIp, lanIp, systemKind, commit, SEID))
                              {
                                    (*g_log).writeln("Register server,SEID:" + SEID +
                                                     "\nwan ip:" + wanIp +
                                                     "\nlan ip:" + lanIp +
                                                     "\nsystemKind:" + (systemKind == 0 ? "Windows" : "Linux") +
                                                     "\ncommit:" + commit);
                                    send(aptSocket, "OK");
                                    send(aptSocket, SEID);
                                    serverThreadArry.push_back(std::thread(serverThread, SEID));
                              }
                              else
                                    goto failRegister;
                        }
                        else
                              goto failRegister;
                        break;
                  case 'H': // Healthy socket connect
                        if ((*ClientInfo).find(buf.substr(1)) != (*ClientInfo).end())
                        {
                              (*g_log).writeln("Client healthy socket connect,SEID:" + buf.substr(1));
                              *(*ClientInfo)[buf.substr(1)]->healthSocket = aptSocket;
                              healthyBeatSOCKETList.push_back(healthyBeatInfoStruct(buf, aptSocket));
                              send(aptSocket, "OK");
                        }
                        else if ((*ServerInfo).find(buf.substr(1)) != (*ServerInfo).end())
                        {
                              (*g_log).writeln("Server healthy socket connect,SEID:" + buf.substr(1));
                              *(*ServerInfo)[buf.substr(1)]->healthSocket = aptSocket;
                              healthyBeatSOCKETList.push_back(healthyBeatInfoStruct(buf, aptSocket, true));
                              send(aptSocket, "OK");
                        }
                        else
                              goto failRegister;
                        break;
                  failRegister:
                        send(aptSocket, "FAIL");
                        closesocket(aptSocket);
                  }
            }
      }
}