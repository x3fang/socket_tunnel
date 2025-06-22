#include "globalDefine.h"
struct programPluginInfoStruct
{
      std::shared_ptr<SOCKET> mainConnectSocket;
      std::shared_ptr<SOCKET> healthySocket;
      std::shared_ptr<std::string> connectIp;
      int connectPort;
      programPluginInfoStruct()
          : mainConnectSocket(::mainConnectSocket), healthySocket(::healthySocket),
            connectIp(::connectIp), connectPort(::connectPort) {}
      programPluginInfoStruct(const programPluginInfoStruct &other)
          : mainConnectSocket(other.mainConnectSocket),
            healthySocket(other.healthySocket), connectIp(other.connectIp),
            connectPort(other.connectPort) {}
      programPluginInfoStruct &operator=(const programPluginInfoStruct &other)
      {
            connectIp = other.connectIp;
            connectPort = other.connectPort;
            mainConnectSocket = other.mainConnectSocket;
            healthySocket = other.healthySocket;
            return *this;
      }
};