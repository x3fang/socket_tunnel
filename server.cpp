#define initClientSocket_
#include "include\globalDefine.h"
#include "include\log.h"
#include "include\Plugin.h"
#include "include\serverStruct.h"
#include <iostream>
// using namespace logNameSpace;
using logNameSpace::Log;
WSADATA g_wsaData;
sockaddr_in g_sockaddr;
std::thread healthyBeatThread;
bool stopSign = false;
void healthyBeat(SOCKET &sock)
{
      while (DEBUG)
            ;
      auto prlog = g_log.getFunLog("healthyBeat");
      int timeout = 3000;
      setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
      setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout));
      std::string buf;
      while (true)
      {
            int res1 = recv(sock, buf);
            int res2 = send(sock, buf);
            if (buf == "del")
            {
                  prlog->writeln("server del us,program exit");
                  break;
            }
            else if (res1 == SOCKET_ERROR || res2 == SOCKET_ERROR)
            {
                  prlog->writeln("recv error,program exit");
                  break;
            }
      }
      stopSign = true;
      closesocket(sock);
      return;
}
std::string getLanIp()
{
      SECURITY_ATTRIBUTES sa = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
      HANDLE hReadPipe, hWritePipe;
      if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0))
            return "127.0.0.1";

      // 合并命令：先修改代码页为UTF-8，再执行ipconfig
      const char *cmd = "cmd.exe /c \"chcp 65001 >nul && ipconfig\"";

      STARTUPINFOA si = {sizeof(STARTUPINFOA)};
      si.dwFlags = STARTF_USESTDHANDLES;
      si.hStdOutput = hWritePipe;
      si.hStdError = hWritePipe;

      PROCESS_INFORMATION pi;
      if (!CreateProcessA(NULL, (LPSTR)cmd, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
      {
            CloseHandle(hReadPipe);
            CloseHandle(hWritePipe);
            return "127.0.0.1";
      }

      CloseHandle(hWritePipe);
      std::string output;
      char buffer[4096];
      DWORD bytesRead;
      while (ReadFile(hReadPipe, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0)
      {
            output.append(buffer, bytesRead);
      }

      CloseHandle(hReadPipe);
      WaitForSingleObject(pi.hProcess, INFINITE);
      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);
      output = output.substr(output.find("IPv4 Address") + 12);
      output = output.substr(output.find_first_of("0123456789"));
      output = output.substr(0, output.find_first_not_of("0123456789."));
      return output;
}
std::vector<std::string> recvAndMatchPluginList(SOCKET &sock)
{
      std::string buf;
      std::vector<std::string> pluginList;
      while (buf != "end")
      {
            recv(*mainConnectSocket, buf);
            if (buf != "end" && pluginManager.findFun(buf))
                  pluginList.push_back(buf);
      }
      return pluginList;
}
int main()
{
      g_log.setName("server");
      g_log.writeln("program start");
      (*connectIp) = "127.0.0.1";
      connectPort = 6020;
      PluginNamespace::loadPlugin(pluginManager, ".\\server_plugin\\");
      initClientSocket(g_wsaData, *mainConnectSocket, g_sockaddr, *connectIp, connectPort);
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
      std::string helloMsg = "S" + getLanIp() + "W" + "This is a test!";
#endif
      while (connect(*mainConnectSocket, (sockaddr *)&g_sockaddr, sizeof(g_sockaddr)))
            ;
      send(*mainConnectSocket, helloMsg);
      recv(*mainConnectSocket, helloMsg);
      if (helloMsg == "OK")
      {
            recv(*mainConnectSocket, helloMsg);
            initClientSocket(g_wsaData, *healthySocket, g_sockaddr, *connectIp, connectPort);
            while (connect(*healthySocket, (sockaddr *)&g_sockaddr, sizeof(g_sockaddr)))
                  ;
            send(*healthySocket, 'H' + helloMsg);
            recv(*healthySocket, helloMsg);
            if (helloMsg == "OK")
            {
                  healthyBeatThread = std::thread(healthyBeat, std::ref(*healthySocket));
                  std::string buf;
                  while (!stopSign)
                  {
                        system("cls");
                        auto temp = recvAndMatchPluginList(*mainConnectSocket);
                        int index = 0, inputIndex = 0;
                        for (auto &it : temp)
                              std::cout << "[ " << index++ << " ] pluginName:" << it << "\n";
                        std::cout << "input plugin index:";
                        inputIndex = 0;
                        // std::cin >> inputIndex;
                        if (inputIndex >= 0 && inputIndex < temp.size())
                              send(*mainConnectSocket, temp[inputIndex]);
                        recv(*mainConnectSocket, buf);
                        if (buf == "ok")
                        {
                              PluginInfo info;
                              info.cus = std::make_shared<pluginInfo>();
                              info.cus->data.push_back(std::make_shared<PluginInfoStruct>());
                              pluginManager.runFun(temp[inputIndex], info);
                        }
                        else
                        {
                              std::cout << "Don't found plugin or run failed";
                        }
                  }
            }
      }
      WSACleanup();
      return 0;
}