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

#include <algorithm>
#include <cstring>
#include <string>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#define SUCCESS_OPERAT 0
std::shared_ptr<std::string> connectIp = std::make_shared<std::string>();
int connectPort;

std::shared_ptr<SOCKET> mainConnectSocket = std::make_shared<SOCKET>();
std::shared_ptr<SOCKET> healthySocket = std::make_shared<SOCKET>();

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
      std::string resData;
      char buf[1024] = {0};
      while (true)
      {
            int res = recv(sock, buf, 2, 0);
            if (res == SOCKET_ERROR)
                  return WSAGetLastError();
            if (res == 2 && buf[0] == '\r' && buf[2] == '\n')
            {
                  res = recv(sock, buf, sizeof(buf), 0);
                  if (res == SOCKET_ERROR)
                        return WSAGetLastError();
                  resData += std::string(buf, res);
                  if (resData.find("\r\n") != std::string::npos)
                  {
                        data = resData.substr(0, resData.find("\r\n"));
                        return SUCCESS_OPERAT;
                  }
            }
      }
}
#endif