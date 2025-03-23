#ifndef CLIENTDEFINE_H
#define CLIENTDEFINE_H
#include <winsock2.h>
#include <ws2tcpip.h>
// #include <windows.h>
#include <stdio.h>
#include <iostream>
#include <thread>
#include <random>
#include <time.h>
#include <string>
#include <string.h>
#include <winternl.h>
#include <intrin.h>
#include <fstream>
#include <atomic>
#include <sstream>
#include <map>

// #pragma comment(lib, "ws2_32")
int localPort;
std::string ip;
int serverPort;
bool ServerState = false;
std::string SEID;
SOCKET HealthyBeat;
SOCKET s;
WSADATA wsaData;
SOCKADDR_IN addr, addr2;

struct clientPluginInfo
{
      int localPort;
      std::string ip;
      int serverPort;
      std::string SEID;
      bool *ServerState;
      SOCKET *HealthyBeat;
      SOCKET *s;
      SOCKADDR_IN *addr;
      SOCKADDR_IN *addr2;
      std::string msg;
      clientPluginInfo()
      {
            this->localPort = ::localPort;
            this->ip = ::ip;
            this->serverPort = ::serverPort;
            this->SEID = ::SEID;
            this->ServerState = &::ServerState;
            this->HealthyBeat = &::HealthyBeat;
            this->s = &::s;
            this->addr = &::addr;
            this->addr2 = &::addr2;
      }
      clientPluginInfo &operator=(const clientPluginInfo &right)
      {
            this->localPort = right.localPort;
            this->ip = right.ip;
            this->serverPort = right.serverPort;
            this->SEID = right.SEID;
            this->ServerState = right.ServerState;
            this->HealthyBeat = right.HealthyBeat;
            this->s = right.s;
            this->addr = right.addr;
            this->addr2 = right.addr2;
            return *this;
      }
      clientPluginInfo(const std::string &msg)
      {
            this->msg = msg;
            this->localPort = ::localPort;
            this->ip = ::ip;
            this->serverPort = ::serverPort;
            this->SEID = ::SEID;
            this->ServerState = &::ServerState;
            this->HealthyBeat = &::HealthyBeat;
            this->s = &::s;
            this->addr = &::addr;
            this->addr2 = &::addr2;
      }
      ~clientPluginInfo()
      {
            this->ServerState = nullptr;
            this->HealthyBeat = nullptr;
            this->s = nullptr;
            this->addr = nullptr;
            this->addr2 = nullptr;
      }
};
bool send_message(SOCKET sock, const std::string &message);
bool receive_message(SOCKET sock, std::string &message);
#include "clientDefine.cpp"
#endif