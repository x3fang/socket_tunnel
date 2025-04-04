#include "include\globalDefine.h"
#include "include\log.h"
#include "include\Plugin.h"
using namespace logNameSpace;
using PluginNamespace::pluginInfo;
using PluginNamespace::PluginInfo;
PluginNamespace::PluginManager pluginManager;
Log g_log;
WSADATA g_wsaData;
sockaddr_in g_sockaddr;
int initClientSocket(WSADATA &g_wsaData, SOCKET &sock, sockaddr_in &serverInfo, string serverIP, int serverPort)
{
      int iResult;
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
int main()
{
      g_log.setName("server");
      g_log.writeln("program start");
      (*connectIp) = "127.0.0.1";
      connectPort = 6020;
      PluginNamespace::loadPlugin(".\\server_plugin\\");
}