#include "globalDefine.h"
struct programPluginInfoStruct
{
      std::shared_ptr<SOCKET> mainConnectSocket;
      std::shared_ptr<SOCKET> healthySocket;
      std::shared_ptr<std::string> connectIp;
      std::string SEID;
      int connectPort;
      programPluginInfoStruct()
          : mainConnectSocket(::mainConnectSocket), healthySocket(::healthySocket),
            connectIp(::connectIp), connectPort(::connectPort) {}
      programPluginInfoStruct(const programPluginInfoStruct &other)
          : mainConnectSocket(other.mainConnectSocket),
            healthySocket(other.healthySocket), connectIp(other.connectIp),
            connectPort(other.connectPort), SEID(other.SEID) {}
      programPluginInfoStruct &operator=(const programPluginInfoStruct &other)
      {
            connectIp = other.connectIp;
            connectPort = other.connectPort;
            mainConnectSocket = other.mainConnectSocket;
            healthySocket = other.healthySocket;
            SEID = other.SEID;
            return *this;
      }
};