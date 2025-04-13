#ifndef globalDefine_h
#define globalDefine_h

// windows
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <WinSock2.h>
#include <conio.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")
#endif
#define DEBUG 1
#include <algorithm>
#include <cstring>
#include <string>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <iostream>
#include "log.h"
#define EXPORT __declspec(dllexport)
#define SUCCESS_OPERAT 0
using namespace logNameSpace;
std::shared_ptr<std::string> connectIp = std::make_shared<std::string>();
int connectPort;
std::shared_ptr<SOCKET> mainConnectSocket = std::make_shared<SOCKET>();
std::shared_ptr<SOCKET> healthySocket = std::make_shared<SOCKET>();
Log g_log;
template <typename T>
struct Info
{
      std::shared_ptr<T> cus;
      std::shared_ptr<SOCKET> mainConnectSocket;
      std::shared_ptr<SOCKET> healthySocket;
      std::shared_ptr<std::string> connectIp;
      int connectPort;
      Info()
          : mainConnectSocket(::mainConnectSocket), healthySocket(::healthySocket),
            connectIp(::connectIp), connectPort(::connectPort) {}
      Info(const T &a_cus)
          : cus(a_cus), mainConnectSocket(::mainConnectSocket),
            healthySocket(::healthySocket), connectIp(::connectIp),
            connectPort(::connectIp) {}
      Info(const Info &other)
          : cus(other.cus), mainConnectSocket(other.mainConnectSocket),
            healthySocket(other.healthySocket), connectIp(other.connectIp),
            connectPort(other.connectIp) {}
      Info &operator=(const Info &other)
      {
            *this.cus = other.cus;
            *this.connectIp = other.connectIp;
            *this.connectPort = other.connectPort;
            *this.mainConnectSocket = other.mainConnectSocket;
            *this.healthySocket = other.healthySocket;
      }
};
int send(SOCKET &sock, const std::string &data)
{
      std::string temp("\r\n" + data + "\r\n");
      int res = send(sock, temp.c_str(), temp.size(), 0);
      if (res == SOCKET_ERROR)
            return false;
      return true;
}
int recv(SOCKET &sock, std::string &data)
{
      try
      {
            std::string resData;
            char buf[1024] = {0};
            while (true)
            {
                  int res = recv(sock, buf, 2, 0);
                  if (res == SOCKET_ERROR)
                        return WSAGetLastError();
                  if (res == 2 && strlen(buf) == 2 && buf[0] == '\r' && buf[1] == '\n')
                  {
                        char lastch = 0, thisch = 0;
                        while (lastch != '\r' || thisch != '\n')
                        {
                              res = recv(sock, &thisch, 1, 0);
                              if (res == SOCKET_ERROR)
                                    return WSAGetLastError();
                              resData += thisch;
                              if (lastch == '\r' && thisch == '\n')
                                    break;
                              lastch = thisch;
                        }

                        if (resData.find("\r\n") != std::string::npos)
                        {
                              data = resData.substr(0, resData.find("\r\n"));
                              return SUCCESS_OPERAT;
                        }
                  }
            }
      }
      catch (const std::exception &e)
      {
            std::cout << e.what() << '\n';
            return WSAGetLastError();
      }
}
#ifdef initClientSocket_
int initClientSocket(WSADATA &wsaData, SOCKET &sock, sockaddr_in &serverInfo, std::string &serverIP, int serverPort)
{
      int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
      // Create socket
      sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
      if (sock == INVALID_SOCKET)
      {
            printf("Error at socket(): %ld\n", WSAGetLastError());
            WSACleanup();
            return WSAGetLastError();
      }
      // Set address and port

      serverInfo.sin_family = AF_INET;
      serverInfo.sin_addr.s_addr = inet_addr(serverIP.c_str());
      serverInfo.sin_port = htons(serverPort);
      return 0;
}
#endif
#include "Plugin.h"
PluginNamespace::PluginManager pluginManager;
using PluginNamespace::PluginInfo;
using PluginNamespace::pluginInfo;
#endif