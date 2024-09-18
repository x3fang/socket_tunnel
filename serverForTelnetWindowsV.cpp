#pragma comment(lib, "ws2_32.lib")
// 设置连接器选项
#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <random>
#include <sstream>
#include <fstream>
#include <thread>
#include <time.h>
#include <atomic>
#include <algorithm>
#include <conio.h>
#include <map>
#include <queue>
#include "..\\MD5.h"
#include "getCmd.h"
using std::atomic;
using std::cin;
using std::cout;
using std::endl;
using std::fstream;
using std::ifstream;
using std::ios;
using std::istringstream;
using std::map;
using std::ofstream;
using std::queue;
using std::string;
using std::thread;
using std::to_string;
using std::vector;
typedef struct SEIDForSocketStruct
{
    SOCKET socketH;
    bool isSocketExit;
    bool isSEIDExit;
    bool isBack;
    bool isUse;
    string SEID;
    SOCKET ServerSocket;
    atomic<bool> serverSocketLock;
    atomic<bool> serverHealthySocketLock;
    atomic<bool> otherValueLock;

    SEIDForSocketStruct()
    {
        SEID.clear();
        ServerSocket = INVALID_SOCKET;
        socketH = INVALID_SOCKET;
        isSEIDExit = false;
        isSocketExit = false;
        isBack = false;
        isUse = false;
        serverSocketLock.exchange(false, std::memory_order_relaxed);
        serverHealthySocketLock.exchange(false, std::memory_order_relaxed);
        otherValueLock.exchange(false, std::memory_order_relaxed);
    }
    void getServerSocketLock()
    {
        while (serverSocketLock.exchange(true, std::memory_order_acquire))
            ;
    }
    void releaseServerSocketLock()
    {
        serverSocketLock.exchange(false, std::memory_order_release);
    }
    void getServerHealthySocketLock()
    {
        while (serverHealthySocketLock.exchange(true, std::memory_order_acquire))
            ;
    }
    void releaseServerHealthySocketLock()
    {
        serverHealthySocketLock.exchange(false, std::memory_order_release);
    }
    void getOtherValueLock()
    {
        while (otherValueLock.exchange(true, std::memory_order_acquire))
            ;
    }
    void releaseOtherValueLock()
    {
        otherValueLock.exchange(false, std::memory_order_release);
    }
};
typedef struct ClientSocketFlagStruct
{
    SOCKET ClientSocket;
    string ClientWanIp;
    string ClientLanIp;
    unsigned long long int OnlineTime, OfflineTime;
    int ClientConnectPort;
    enum states
    {
        NULLs = 0,
        Online = 1,
        Offline = 2,
        Use = 3
    };
    states state;
    bool operator==(const ClientSocketFlagStruct &e)
    {
        return (this->ClientLanIp == e.ClientLanIp && this->ClientWanIp == e.ClientWanIp);
    }
};
class filter
{
private:
    bool notMatching;
    struct rule
    {

        string ruleData;
        int ruleOperatorType; //!= == > < >= <=
                              // 1  2 3 4 5  6
        rule(string _ruleData, int _ruleOperatorType)
        {
            ruleData = _ruleData;
            ruleOperatorType = _ruleOperatorType;
        }
    };
    vector<rule> wanIpRule, lanIpRule, portRule;
    map<string, int> StringToIntForruleOperator =
        {
            {"!=", 1},
            {"==", 2},
            {">", 3},
            {"<", 4},
            {">=", 5},
            {"<=", 6}};
    bool GetnotMatchingState()
    {
        return this->notMatching;
    }

public:
    enum ruleDataType
    {
        wanip = 1,
        lanip = 2,
        port = 3,
        all = 4
    };
    const bool matching(string wanIp, string lanIp, string port)
    {
        if (notMatching)
        {
            return true;
        }
        bool wanIpMatch = false;
        bool lanIpMatch = false;
        bool portMatch = false;

        if ((wanIpRule.size() == 0 && lanIpRule.size() == 0 && portRule.size() == 0)

            || (wanIp.empty() && lanIp.empty() && port.empty()))
        {
            return false;
        }
        // wanIp matching
        if (wanIpRule.size() > 0 && !wanIp.empty())
            for (int i = 1; i <= wanIpRule.size(); i++)
            {
                if (wanIp.find(wanIpRule[i].ruleData) != string::npos)
                {
                    if (wanIpRule[i].ruleOperatorType == 2)
                    {
                        wanIpMatch = true;
                        break;
                    }
                    else
                    {
                        return false;
                    }
                }
            }
        else if (wanIpRule.size() > 0 && wanIp.empty())
            return false;
        else
            wanIpMatch = true;

        // lanIp matching
        if (lanIpRule.size() > 0 && !lanIp.empty())
            for (int i = 1; i <= lanIpRule.size(); i++)
            {
                if (lanIp.find(lanIpRule[i - 1].ruleData) != string::npos)
                {
                    if (lanIpRule[i].ruleOperatorType == 2)
                    {
                        lanIpMatch = true;
                        break;
                    }
                    else
                    {
                        return false;
                    }
                }
            }
        else if (lanIpRule.size() > 0 && lanIp.empty())
            return false;
        else
            lanIpMatch = true;

        // port matching
        if (portRule.size() > 0 && !portRule.empty())
            for (int i = 1; i <= portRule.size(); i++)
            {
                int iPort = stoi(port);
                int iPortRule = stoi(portRule[i - 1].ruleData);
                switch (portRule[i - 1].ruleOperatorType)
                {
                case 1: //!=
                    if (port == portRule[i - 1].ruleData)
                        return false;
                    portMatch = true;
                    break;
                case 2: //==
                    if (port == portRule[i - 1].ruleData)
                        portMatch = true;
                    else
                        return false;
                    break;
                case 3: //>
                    if (iPort > iPortRule)
                        portMatch = true;
                    else
                        return false;
                    break;
                case 4: //<
                    if (iPort < iPortRule)
                        portMatch = true;
                    else
                        return false;
                case 5: //>=
                    if (iPort >= iPortRule)
                        portMatch = true;
                    else
                        return false;
                case 6: //<=
                    if (iPort <= iPortRule)
                        portMatch = true;
                    else
                        return false;
                }
            }
        else if (portRule.size() > 0 && portRule.empty())
            return false;
        else
            portMatch = true;
        return wanIpMatch && lanIpMatch && portMatch;
    }
    bool addRule(ruleDataType ruleDataType, string ruleOperatorType, string ruleData)
    {
        vector<rule> *ruleVector;
        switch (ruleDataType)
        {
        case wanip:
            if (ruleOperatorType == "!=" || ruleOperatorType == "==")
            {
                ruleVector = &wanIpRule;
                break;
            }
            else
                return false;
        case lanip:
            if (ruleOperatorType == "!=" || ruleOperatorType == "==")
            {
                ruleVector = &lanIpRule;
                break;
            }
            else
                return false;
        case port:
            if (ruleOperatorType == "!=" || ruleOperatorType == "==" || ruleOperatorType == ">" || ruleOperatorType == "<" || ruleOperatorType == ">=" || ruleOperatorType == "<=")
            {
                ruleVector = &portRule;
                break;
            }
            else
                return false;
        case all:
            notMatching = true;
            return true;
        default:
            return false;
        }
        ruleVector->push_back(rule{ruleData, StringToIntForruleOperator[ruleOperatorType]});
        ruleVector = NULL;
        return true;
    }
};
map<string, int> StringToInt =
    {
        {"connect", 1},
        {"del", 2},
        {"show", 3},
        {"cmd", 4}

};
WSADATA wsaData;
SOCKET ListenSocket;
sockaddr_in service;
queue<SOCKET> ClientSocketQueue, ServerSocketQueue;
map<string, SEIDForSocketStruct> ServerSEIDMap, ClientSEIDMap;
vector<ClientSocketFlagStruct> DataSaveArry;
vector<ClientSocketFlagStruct> ClientMap;
string password;
bool dataIsChange = false;
vector<thread> ServerRSThreadArry, ClientRSThreadArry;
atomic<bool> ClientMapLock(false);
atomic<bool> ServerQueueLock(false), ClientQueueLock(false);
bool ischange = false;
int PassDataInit = 0;
BOOL WINAPI HandlerRoutine(DWORD dwCtrlType);
int initServer(SOCKET &, sockaddr_in &, int);
string StringTime(time_t);
void HealthyCheckByServer(string);
void HealthyCheckByClient(string);
int showForSend(string, filter, bool, ClientSocketFlagStruct::states);
void Connect(string, vector<string>, int);
int delForId(int);
void del(string, vector<string>, int);
void show(string, vector<string>, int);
void cmod(string, vector<string>, int);
string createSEID(SOCKET, string);
void joinClient(string, string, string, SOCKET, unsigned long long int, unsigned long long int);
void ServerRS(SOCKET);
void ClientRS(SOCKET);
void ServerConnect();
void ClientConnect();
void saveData();
void dataSave();
void passData();
void loadData();
bool send_message(SOCKET sock, const std::string &message);
bool receive_message(SOCKET sock, std::string &message);
int main(int, char **);

BOOL WINAPI HandlerRoutine(DWORD dwCtrlType)
{
    switch (dwCtrlType)
    {
        // case CTRL_CLOSE_EVENT: // 关闭
        //     End();
        //     break;
        // case CTRL_LOGOFF_EVENT: // 用户退出
        //     End();
        //     break;
        // case CTRL_SHUTDOWN_EVENT: // 系统被关闭时.
        //     End();
        //     break;
    }
    return TRUE;
}
int initServer(SOCKET &ListenSocket, sockaddr_in &sockAddr, int port)
{
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
        return iResult;
    // 创建套接字
    ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ListenSocket == INVALID_SOCKET)
    {
        WSACleanup();
        return WSAGetLastError();
    }
    // 绑定套接字

    sockAddr.sin_family = AF_INET;
    sockAddr.sin_addr.s_addr = INADDR_ANY;
    sockAddr.sin_port = htons(port);
    if (bind(ListenSocket, (SOCKADDR *)&sockAddr, sizeof(sockAddr)) == SOCKET_ERROR)
    {
        closesocket(ListenSocket);
        WSACleanup();
        return WSAGetLastError();
    }
    // 监听套接字
    if (listen(ListenSocket, 5) == SOCKET_ERROR)
    {
        closesocket(ListenSocket);
        WSACleanup();
        return WSAGetLastError();
    }
    return 0;
}
string StringTime(time_t t1)
{
    time_t t = t1;
    char tmp[64];
    struct tm *timinfo;
    timinfo = localtime(&t);

    strftime(tmp, sizeof(tmp), "%Y%m%d%H%M", timinfo);
    return tmp;
}
void HealthyCheckByServer(const string SEID)
{
    cout << "12";
    int timeout = 1000; // 设置超时时间为 10 秒
    ServerSEIDMap[SEID].getServerHealthySocketLock();
    setsockopt(ServerSEIDMap[SEID].socketH, SOL_SOCKET, SO_RCVTIMEO, (char *)timeout, sizeof(timeout));
    setsockopt(ServerSEIDMap[SEID].socketH, SOL_SOCKET, SO_SNDTIMEO, (char *)timeout, sizeof(timeout));
    ServerSEIDMap[SEID].releaseServerHealthySocketLock();
    while (1)
    {
        srand(time(NULL));
        string sendMsg = to_string(rand() % 1000000000);
        string buf;
        ServerSEIDMap[SEID].getServerHealthySocketLock();
        int state = send_message(ServerSEIDMap[SEID].socketH, sendMsg);
        int state1 = receive_message(ServerSEIDMap[SEID].socketH, buf);
        if (strcmp(buf.c_str(), "\r\nClose\r\n") == 0)
        {
            ServerSEIDMap[SEID].getOtherValueLock();
            ServerSEIDMap[SEID].isBack = true;
            ServerSEIDMap[SEID].isSocketExit = false;
            ServerSEIDMap[SEID].releaseOtherValueLock();
            closesocket(ServerSEIDMap[SEID].socketH);
            ServerSEIDMap[SEID].releaseServerHealthySocketLock();
            cout << "unhelathy";
            return;
        }
        if (strcmp(buf.c_str(), sendMsg.c_str()) != 0 || ServerSEIDMap[SEID].isBack || state <= 0 || state1 <= 0)
        {
            ServerSEIDMap[SEID].getOtherValueLock();
            ServerSEIDMap[SEID].isBack = true;
            ServerSEIDMap[SEID].isSocketExit = false;
            ServerSEIDMap[SEID].releaseOtherValueLock();
            closesocket(ServerSEIDMap[SEID].socketH);
            ServerSEIDMap[SEID].releaseServerHealthySocketLock();
            return;
        }
        ServerSEIDMap[SEID].releaseServerHealthySocketLock();
        Sleep(1000);
    }
}
void HealthyCheckByClient(string SEID)
{
    int timeout = 10000; // 设置超时时间为 10 秒
    ClientSEIDMap[SEID].getServerHealthySocketLock();
    setsockopt(ClientSEIDMap[SEID].socketH, SOL_SOCKET, SO_RCVTIMEO, (char *)timeout, sizeof(timeout));
    setsockopt(ClientSEIDMap[SEID].socketH, SOL_SOCKET, SO_SNDTIMEO, (char *)timeout, sizeof(timeout));
    ClientSEIDMap[SEID].getServerHealthySocketLock();
    while (1)
    {
        srand(time(NULL));
        string sendMsg = to_string(rand() % 1000000000);
        string buf;
        ClientSEIDMap[SEID].getServerHealthySocketLock();
        int state1 = send_message(ClientSEIDMap[SEID].socketH, sendMsg);
        int state2 = receive_message(ClientSEIDMap[SEID].socketH, buf);
        if (strcmp(buf.c_str(), sendMsg.c_str()) != 0 || ClientSEIDMap[SEID].isBack || state2 <= 0 || state1 <= 0)
        {
            ClientSEIDMap[SEID].getOtherValueLock();
            ClientSEIDMap[SEID].isBack = true;
            ClientSEIDMap[SEID].isSocketExit = false;
            ClientSEIDMap[SEID].releaseOtherValueLock();
            closesocket(ClientSEIDMap[SEID].socketH);
            cout << "unhelathy";
            ServerSEIDMap[SEID].releaseServerHealthySocketLock();
            return;
        }
        ClientSEIDMap[SEID].getServerHealthySocketLock();
    }
}
int showForSend(string seid, filter f, bool startIf = false, ClientSocketFlagStruct::states state = ClientSocketFlagStruct::NULLs)
{
    SOCKET &s = ServerSEIDMap[seid].ServerSocket;
    int canShowClient = 0;
    while (ClientMapLock.exchange(true, std::memory_order_acquire))
        ; // 加锁
    for (int i = 1; i <= ClientMap.size(); i++)
    {

        if (ServerSEIDMap[seid].isBack)
            return -1;
        if (!startIf || ClientMap[i - 1].state == state || f.matching(ClientMap[i - 1].ClientWanIp, ClientMap[i - 1].ClientLanIp, to_string(ClientMap[i - 1].ClientConnectPort)))
        {
            canShowClient++;
            string sendBuf = ClientMap[i - 1].ClientWanIp + " " + ClientMap[i - 1].ClientLanIp + " " + to_string(ClientMap[i - 1].ClientConnectPort) + " " + to_string(ClientMap[i - 1].state);
            ServerSEIDMap[seid].getServerSocketLock();
            send_message(s, sendBuf);
            ServerSEIDMap[seid].releaseServerSocketLock();
        }
    }
    ClientMapLock.exchange(false, std::memory_order_release); // 解锁
    ServerSEIDMap[seid].getServerSocketLock();
    send_message(s, "\r\n\r\nend\r\n\r\n");
    ServerSEIDMap[seid].releaseServerSocketLock();
    return canShowClient;
}
void Connect(string seid, vector<string> cmods, int cmodsNum)
{
    SOCKET &s = ServerSEIDMap[seid].ServerSocket;
    string recvBuf;
    while (1)
    {
        ServerSEIDMap[seid].getOtherValueLock();
        if (ServerSEIDMap[seid].isBack)
            break;
        ServerSEIDMap[seid].releaseOtherValueLock();
        filter temp;
        showForSend(seid, temp, true, ClientSocketFlagStruct::Online);
        ServerSEIDMap[seid].getServerSocketLock();
        int state = receive_message(s, recvBuf);
        ServerSEIDMap[seid].releaseServerSocketLock();
        if (state == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK)
        {
            break;
        }
        else if (state > 0)
        {
            if (strcmp(recvBuf.c_str(), "\r\nexit\r\n") == 0)
            {
                break;
            }
            else if (strcmp(recvBuf.c_str(), "\r\nnext\r\n"))
            {
                continue;
            }
            int setClientId = atoi(recvBuf.c_str());
            SOCKET ClientSocket = INVALID_SOCKET;
            for (int i = 0; i < ClientMap.size(); i++)
            {
                if (ClientMap[i].state == ClientSocketFlagStruct::Online)
                {
                    setClientId--;
                    if (setClientId == 0)
                    {
                        ClientMap[i].state = ClientSocketFlagStruct::Use;
                        ClientSocket = ClientMap[i].ClientSocket;
                        break;
                    }
                }
            }

            if (ClientSocket != INVALID_SOCKET)
            {
                ServerSEIDMap[seid].getServerSocketLock();
                send_message(s, "\r\n\r\nsec\r\n\r\n");
                ServerSEIDMap[seid].releaseServerSocketLock();
                while (1)
                {
                    string buf;
                    ServerSEIDMap[seid].getServerSocketLock();
                    int state = receive_message(s, buf);
                    ServerSEIDMap[seid].releaseServerSocketLock();
                    if (state == SOCKET_ERROR)
                    {
                        break;
                    }
                    else if (state > 0)
                    {
                        if (strcmp(buf.c_str(), "\r\nfexit\r\n") == 0)
                        {
                            send_message(ClientSocket, "exit\r\n");
                            break;
                        }
                        else
                        {
                            send_message(ClientSocket, buf);
                        }
                    }
                }
                break;
            }
            else
            {
                ServerSEIDMap[seid].getServerSocketLock();
                send_message(s, "\r\n\r\nfail\r\n\r\n");
                ServerSEIDMap[seid].releaseServerSocketLock();
            }
        }
    }
}
int delForId(int ClientId)
{
    while (ClientMapLock.exchange(true, std::memory_order_acquire))
        ; // 加锁
    SOCKET ClientSocket = ClientMap[ClientId - 1].ClientSocket;
    ClientMapLock.exchange(false, std::memory_order_release); // 解锁
    if (ClientSocket != INVALID_SOCKET && ClientMap[ClientId - 1].state != ClientSocketFlagStruct::Use)
    {
        if (send_message(ClientSocket, "del") != SOCKET_ERROR)
        {
            return 0;
        }
        else
        {
            return 1;
        }
    }
    else if (ClientSocket != INVALID_SOCKET && ClientMap[ClientId - 1].state == ClientSocketFlagStruct::Use)
    {
        return 2;
    }
    return -1;
}
void del(string seid, vector<string> cmods, int cmodsNum)
{
    SOCKET &s = ServerSEIDMap[seid].ServerSocket;
    string recvBuf;
    while (1)
    {
        ServerSEIDMap[seid].getOtherValueLock();
        if (ServerSEIDMap[seid].isBack)
            break;
        ServerSEIDMap[seid].releaseOtherValueLock();
        filter temp;
        showForSend(seid, temp);
        ServerSEIDMap[seid].getServerSocketLock();
        int state = receive_message(s, recvBuf);
        ServerSEIDMap[seid].releaseServerSocketLock();
        if (state == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK)
        {
            break;
        }
        else if (state > 0)
        {
            if (strcmp(recvBuf.c_str(), "\r\nexit\r\n") == 0)
            {
                break;
            }
            else if (strcmp(recvBuf.c_str(), "\r\nnext\r\n") == 0)
            {
                continue;
            }
            int setClientId = atoi(recvBuf.c_str());
            int res = delForId(setClientId);
            ServerSEIDMap[seid].getServerSocketLock();
            if (res == 0)
            {
                send_message(s, "\r\n\r\nok\r\n\r\n");
            }
            else if (res == 2)
            {
                send_message(s, "\r\n\r\nUse\r\n\r\n");
            }
            else
            {
                send_message(s, "\r\n\r\nUnError\r\n\r\n");
            }
            ServerSEIDMap[seid].releaseServerSocketLock();
        }
    }
}
void show(string seid, vector<string> cmods, int cmodsNum)
{
    SOCKET &s = ServerSEIDMap[seid].ServerSocket;
    cout << "SHOW";
    string recvBuf;
    while (1)
    {
        filter temp;
        ServerSEIDMap[seid].getOtherValueLock();
        if (ServerSEIDMap[seid].isBack)
            break;
        ServerSEIDMap[seid].releaseOtherValueLock();
        showForSend(seid, temp);
        ServerSEIDMap[seid].getServerSocketLock();
        int state = receive_message(s, recvBuf);
        ServerSEIDMap[seid].releaseServerSocketLock();
        if (state == SOCKET_ERROR)
        {
            return;
        }
        else if (strcmp(recvBuf.c_str(), "\r\nclose\r\n") == 0)
        {
            return;
        }
    }
}
void cmod(string seid, vector<string> cmods, int cmodsNum)
{
    //[del,cmd,show][all,]
    cout << "1321222q2312";
    SOCKET &s = ServerSEIDMap[seid].ServerSocket;
    cout << "1321222q2312\n";
    map<string, int> StringToIntInComd = {
        {"run", 1},
        {"show", 2},
        {"del", 3},
        {"cmod", 4},

        {"port", 5},
        {"wanip", 6},
        {"lanip", 7},
        {"all", 8}};
    filter f;
    if (strcmp(cmods[1].c_str(), "del") == 0 && cmodsNum >= 3)
    {
        ServerSEIDMap[seid].getOtherValueLock();
        if (ServerSEIDMap[seid].isBack)
            return;
        ServerSEIDMap[seid].releaseOtherValueLock();
        ServerSEIDMap[seid].getServerSocketLock();
        send_message(s, "\r\ndel\r\n");
        ServerSEIDMap[seid].releaseServerSocketLock();
        for (int i = 2; i <= cmodsNum; i += 3)
        {
            ServerSEIDMap[seid].getOtherValueLock();
            if (ServerSEIDMap[seid].isBack)
                return;
            ServerSEIDMap[seid].releaseOtherValueLock();
            ServerSEIDMap[seid].getServerSocketLock();
            switch (StringToIntInComd[cmods[i]])
            {
            case 5:
                if (!f.addRule(filter::ruleDataType::port, cmods[i + 1], cmods[i + 2]))
                {
                    send_message(s, "\r\ncmd error\r\n");
                    ServerSEIDMap[seid].releaseServerSocketLock();
                    return;
                }
                break;
            case 6:
                if (!f.addRule(filter::ruleDataType::wanip, cmods[i + 1], cmods[i + 2]))
                {
                    send_message(s, "\r\ncmd error\r\n");
                    ServerSEIDMap[seid].releaseServerSocketLock();
                    return;
                }
                break;
            case 7:
                if (!f.addRule(filter::ruleDataType::lanip, cmods[i + 1], cmods[i + 2]))
                {
                    send_message(s, "\r\ncmd error\r\n");
                    ServerSEIDMap[seid].releaseServerSocketLock();
                    return;
                }
                break;
            case 8:
                if (!f.addRule(filter::ruleDataType::all, cmods[i + 1], cmods[i + 2]))
                {
                    send_message(s, "\r\ncmd error\r\n");
                    ServerSEIDMap[seid].releaseServerSocketLock();
                    return;
                }
                break;
            }
            ServerSEIDMap[seid].releaseServerSocketLock();
        }
        int sectClient = 0;
        for (int i = 1; i <= ClientMap.size(); i++)
        {
            if (f.matching(ClientMap[i - 1].ClientWanIp,
                           ClientMap[i - 1].ClientLanIp,
                           to_string(ClientMap[i - 1].ClientConnectPort)) &&
                ClientMap[i - 1].state != ClientSocketFlagStruct::Use)
            {
                sectClient++;
                delForId(i);
            }
        }
        if (ServerSEIDMap[seid].isBack)
            return;
        ServerSEIDMap[seid].getServerSocketLock();
        send_message(s, "\r\nok\r\n");
        send_message(s, to_string(sectClient).c_str());
        send_message(s, to_string(ClientMap.size()).c_str());
        ServerSEIDMap[seid].releaseServerSocketLock();
    }
    else if (strcmp(cmods[1].c_str(), "cmd") == 0 && cmodsNum >= 4)
    {
        ServerSEIDMap[seid].getOtherValueLock();
        if (ServerSEIDMap[seid].isBack)
            return;
        ServerSEIDMap[seid].releaseOtherValueLock();
        ServerSEIDMap[seid].getServerSocketLock();
        send_message(s, "\r\ncmd\r\n");
        ServerSEIDMap[seid].releaseServerSocketLock();
        int cmdstart = 0;
        // 一条指令
        // cmd [port [!=,==,>,<,>=,<=] [port]] [wanip [!=,==] [ip]] [lanip [!=,==] [ip]] [all] "指令"
        for (int i = 2; i <= cmodsNum && cmods[i][0] != '"'; i += 3, cmdstart = i)
        {
            ServerSEIDMap[seid].getOtherValueLock();
            if (ServerSEIDMap[seid].isBack)
                return;
            ServerSEIDMap[seid].releaseOtherValueLock();
            if (cmods[i][0] == '"')
            {
                break;
            }
            ServerSEIDMap[seid].getServerSocketLock();
            switch (StringToIntInComd[cmods[i]])
            {
            case 5:
                if (!f.addRule(filter::ruleDataType::port, cmods[i + 1], cmods[i + 2]))
                {
                    send_message(s, "\r\ncmd error\r\n");
                    ServerSEIDMap[seid].releaseServerSocketLock();
                    return;
                }
                break;
            case 6:
                if (!f.addRule(filter::ruleDataType::wanip, cmods[i + 1], cmods[i + 2]))
                {
                    send_message(s, "\r\ncmd error\r\n");
                    ServerSEIDMap[seid].releaseServerSocketLock();
                    return;
                }
                break;
            case 7:
                if (!f.addRule(filter::ruleDataType::lanip, cmods[i + 1], cmods[i + 2]))
                {
                    send_message(s, "\r\ncmd error\r\n");
                    ServerSEIDMap[seid].releaseServerSocketLock();
                    return;
                }
            case 8:
                if (!f.addRule(filter::ruleDataType::all, cmods[i + 1], cmods[i + 2]))
                {
                    send_message(s, "\r\ncmd error\r\n");
                    ServerSEIDMap[seid].releaseServerSocketLock();
                    return;
                }
                break;
            }
            ServerSEIDMap[seid].releaseServerSocketLock();
        }
        if (cmdstart > cmodsNum) // 判断指令格式是否标准
        {
            ServerSEIDMap[seid].getOtherValueLock();
            if (ServerSEIDMap[seid].isBack)
                return;
            ServerSEIDMap[seid].releaseOtherValueLock();
            ServerSEIDMap[seid].getServerSocketLock();
            send_message(s, "\r\ncmd error\r\n");
            ServerSEIDMap[seid].releaseServerSocketLock();
            return;
        }
        string sendBuf;
        int sectClient = 0;
        // 合并指令
        /*
        之前的存储模式：按空格分割
        */
        for (int i = cmdstart; i <= cmodsNum; i++)
        {
            sendBuf += cmods[i] + " ";
        }
        sendBuf = sendBuf.substr(1, sendBuf.length() - 2); // 去掉头尾的 ' " '
        sendBuf += "\r\nexit\r\n";
        ClientMapLock.exchange(true, std::memory_order_acquire); // 加锁
        for (int i = 1; i <= ClientMap.size(); i++)
        {
            if (f.matching(ClientMap[i - 1].ClientWanIp, ClientMap[i - 1].ClientLanIp, to_string(ClientMap[i - 1].ClientConnectPort)) && ClientMap[i - 1].state != ClientSocketFlagStruct::Use)
            {
                sectClient++;
                send_message(ClientMap[i - 1].ClientSocket, sendBuf); // 发送指令至Client
            }
        }
        ClientMapLock.exchange(false, std::memory_order_release); // 解锁
        ServerSEIDMap[seid].getOtherValueLock();
        if (ServerSEIDMap[seid].isBack)
            return;
        ServerSEIDMap[seid].releaseOtherValueLock();
        ServerSEIDMap[seid].getServerSocketLock();
        send_message(s, "\r\nok\r\n");
        send_message(s, to_string(sectClient).c_str());
        send_message(s, to_string(ClientMap.size()).c_str());
        ServerSEIDMap[seid].releaseServerSocketLock();
    }
    else if (strcmp(cmods[1].c_str(), "show") == 0 && cmodsNum >= 3)
    {
        ServerSEIDMap[seid].getOtherValueLock();
        if (ServerSEIDMap[seid].isBack)
            return;
        ServerSEIDMap[seid].releaseOtherValueLock();
        ServerSEIDMap[seid].getServerSocketLock();
        send_message(s, "\r\nsee\r\n");
        cout << "show";
        ServerSEIDMap[seid].releaseServerSocketLock();
        for (int i = 2; i <= cmodsNum && (cmodsNum - i) >= 2; i += 3)
        {
            ServerSEIDMap[seid].getServerSocketLock();
            switch (StringToIntInComd[cmods[i]])
            {
            case 5:
                if (!f.addRule(filter::ruleDataType::port, cmods[i + 1], cmods[i + 2]))
                {
                    send_message(s, "\r\ncmd error\r\n");
                    ServerSEIDMap[seid].releaseServerSocketLock();
                    return;
                }
                break;
            case 6:
                if (!f.addRule(filter::ruleDataType::wanip, cmods[i + 1], cmods[i + 2]))
                {
                    send_message(s, "\r\ncmd error\r\n");
                    ServerSEIDMap[seid].releaseServerSocketLock();
                    return;
                }
                break;
            case 7:
                if (!f.addRule(filter::ruleDataType::lanip, cmods[i + 1], cmods[i + 2]))
                {
                    send_message(s, "\r\ncmd error\r\n");
                    ServerSEIDMap[seid].releaseServerSocketLock();
                    return;
                }
                break;
            case 8:
                if (!f.addRule(filter::ruleDataType::all, NULL, NULL))
                {
                    send_message(s, "\r\ncmd error\r\n");
                    ServerSEIDMap[seid].releaseServerSocketLock();
                    return;
                }
                break;
            default:
                send_message(s, "\r\ncmd error\r\n");
                return;
            }
            ServerSEIDMap[seid].releaseServerSocketLock();
        }
        int showClient = showForSend(seid, f);
        ServerSEIDMap[seid].getOtherValueLock();
        if (ServerSEIDMap[seid].isBack)
            return;
        ServerSEIDMap[seid].releaseOtherValueLock();
        ServerSEIDMap[seid].getServerSocketLock();
        send_message(s, "\r\nok\r\n");
        send_message(s, to_string(showClient).c_str());
        send_message(s, to_string(ClientMap.size()).c_str());
        ServerSEIDMap[seid].releaseServerSocketLock();
    }
}
string createSEID(SOCKET sock, string something = NULL)
{
    MD5 m;
    m.init();
    return m.encode(StringTime(time(NULL)) + to_string(sock));
}
void joinClient(string ClientWanIp, string ClientLanIp, string ClientPort, SOCKET ClientSocket, unsigned long long int OnlineTime, unsigned long long int OfflineTime)
{
    while (ClientMapLock.exchange(true, std::memory_order_acquire))
        ; // 加锁
    ClientSocketFlagStruct temp;
    temp.ClientSocket = ClientSocket;
    temp.ClientWanIp = ClientWanIp;
    temp.ClientLanIp = ClientLanIp;
    temp.ClientConnectPort = atoi(ClientPort.c_str());
    temp.OnlineTime = OnlineTime;
    temp.OfflineTime = OfflineTime;
    temp.state = ClientSocketFlagStruct::states::Online;
    vector<ClientSocketFlagStruct>::iterator itr = find(ClientMap.begin(), ClientMap.end(), temp);
    if (itr != ClientMap.end())
    {
        while (ClientMap[distance(ClientMap.begin(), itr)].state == ClientSocketFlagStruct::states::Use)
            ;
        closesocket(ClientMap[distance(ClientMap.begin(), itr)].ClientSocket);
        ClientMap[distance(ClientMap.begin(), itr)] = temp;
    }
    else
    {
        ClientMap.push_back(temp);
    }
    ClientMapLock.exchange(false, std::memory_order_release); // 解锁
    return;
}
bool send_message(SOCKET sock, const std::string &message)
{
    std::ostringstream oss;
    oss << message.size() << "\r\n"
        << message; // 构建消息，包含长度和实际数据
    std::string formatted_message = oss.str();

    int total_sent = 0;
    int message_length = formatted_message.size();
    const char *data = formatted_message.c_str();

    while (total_sent < message_length)
    {
        int bytes_sent = send(sock, data + total_sent, message_length - total_sent, 0);
        if (bytes_sent == SOCKET_ERROR)
        {
            return false; // 发送失败
        }
        total_sent += bytes_sent;
    }
    return true; // 发送成功
}
bool receive_message(SOCKET sock, std::string &message)
{
    std::string length_str;
    char buffer[1024];
    int received;

    // 首先读取长度部分，直到接收到 \r\n
    while (true)
    {
        received = recv(sock, buffer, 1, 0); // 每次读取一个字节
        if (received <= 0)
        {
            return false; // 连接断开或读取出错
        }
        if (buffer[0] == '\r')
        {
            // 继续读取\n
            received = recv(sock, buffer, 1, 0);
            if (received <= 0 || buffer[0] != '\n')
            {
                return false; // 格式错误
            }
            break; // 读取到 \r\n，退出循环
        }
        length_str += buffer[0];
    }

    int data_length = std::stoi(length_str); // 转换长度字符串为整数
    message.resize(data_length);

    int total_received = 0;
    while (total_received < data_length)
    {
        received = recv(sock, &message[total_received], data_length - total_received, 0);
        if (received <= 0)
        {
            return false; // 连接断开或读取出错
        }
        total_received += received;
    }

    return true; // 接收成功
}
void SetColor(unsigned short forecolor = 4, unsigned short backgroudcolor = 0)
{
    HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);             // 获取缓冲区句柄
    SetConsoleTextAttribute(hCon, forecolor | backgroudcolor); // 设置文本及背景色
}
void ServerRS(SOCKET s)
{
    string lastloginYZM;
    string loginYZM;
    lastloginYZM.clear();
    bool loginTRUE = false;
    MD5 m;
    for (int i = 1; i <= 5 && !loginTRUE; i++)
    {
        m.init();
        string recvBuf;
        receive_message(s, recvBuf);
        loginYZM = m.encode(StringTime(time(NULL)) + lastloginYZM + password);
        cout << loginYZM << endl
             << recvBuf << endl;
        if (recvBuf == loginYZM)
        {
            loginTRUE = true;
            send_message(s, "true");
            break;
        }
        send_message(s, "fail");
        lastloginYZM = loginYZM;
    }
    if (!loginTRUE)
        return;
    cout << "loging OK\n";
    string SEID = createSEID(s, lastloginYZM + loginYZM);
    ServerSEIDMap[SEID].getServerSocketLock();
    send_message(s, SEID.c_str());
    ServerSEIDMap[SEID].releaseServerSocketLock();
    ServerSEIDMap[SEID].getOtherValueLock();
    ServerSEIDMap[SEID].isSEIDExit = true;
    ServerSEIDMap[SEID].SEID = SEID;
    ServerSEIDMap[SEID].ServerSocket = s;
    ServerSEIDMap[SEID].releaseOtherValueLock();
    while (true)
    {
        ServerSEIDMap[SEID].getOtherValueLock();
        if (ServerSEIDMap[SEID].isSocketExit)
        {
            ServerSEIDMap[SEID].releaseOtherValueLock();
            break;
        }
        ServerSEIDMap[SEID].releaseOtherValueLock();
    }
    try
    {
        thread HealthyCheckThread = thread(HealthyCheckByServer, SEID);
        HealthyCheckThread.detach();
    }
    catch (std::exception &e)
    {
        SetColor(4, 0);
        cout << e.what() << endl;
        SetColor(15, 0);
    }

    while (1)
    {
        cout << "Wait for command...\n";
        cout << "start get value lock\n";
        ServerSEIDMap[SEID].getOtherValueLock();
        if (ServerSEIDMap[(string)SEID].isBack)
            cout << "is back\n";
        ServerSEIDMap[SEID].releaseOtherValueLock();
        string recvBuf;
        cout << "start get lock\n";
        ServerSEIDMap[SEID].getServerSocketLock();
        cout << "recv:" << recvBuf;
        int state = receive_message(s, recvBuf);

        ServerSEIDMap[SEID].releaseServerSocketLock();
        if (state == SOCKET_ERROR)
        {
            ServerSEIDMap[SEID].getServerSocketLock();
            ServerSEIDMap[SEID].getOtherValueLock();
            ServerSEIDMap[(string)SEID].isBack = true;
            closesocket(s);
            ServerSEIDMap[SEID].releaseOtherValueLock();
            ServerSEIDMap[SEID].releaseServerSocketLock();
            return;
        }
        else
        {
            vector<string> cmods;
            string token;
            int tokenNum = 0;
            istringstream iss((string)recvBuf);
            while (getline(iss, token, ' '))
            {
                tokenNum++;
                cmods.push_back(token);
            }
            cout << tokenNum << endl;
            cout << cmods[0] << endl;
            cout << StringToInt[cmods[0]];
            switch (StringToInt[cmods[0]])
            {
            case 1:
                Connect(SEID, cmods, tokenNum); // ok
                break;
            case 2:
                del(SEID, cmods, tokenNum); // ok
                break;
            case 3:
                show(SEID, cmods, tokenNum); // ok
                break;
            case 4:
                cmod(SEID, cmods, tokenNum); // ok
                break;
            default:
                break;
            }
        }
    }
}
void ClientRS(SOCKET s)
{
    cout << "ok\n";
    string recvBuf;
    receive_message(s, recvBuf);
    istringstream iss((string)recvBuf);
    string ClientWanIp, ClientLanIp, ClientPort, ClientState;
    iss >> ClientWanIp >> ClientLanIp >> ClientPort;
    joinClient(ClientWanIp, ClientLanIp, ClientPort, s, time(NULL), 0);
    string SEID = createSEID(s, ClientLanIp + ClientWanIp);
    ClientSEIDMap[SEID].isSEIDExit = true;
    send_message(s, SEID);
    cout << "ok\n";
    while (!ClientSEIDMap[(string)SEID].isSocketExit)
        ;
    cout << "ok\n";
    thread HealthyCheckThread = thread(HealthyCheckByClient, SEID);
    while (1)
    {
        if (ServerSEIDMap[(string)SEID].isBack)
        {
            closesocket(s);
            return;
        }
    }
}
void ServerConnect()
{
    while (true)
    {
        while (ServerQueueLock.exchange(true, std::memory_order_acquire))
            ;
        if (ServerSocketQueue.size() > 0)
        {
            SOCKET ServerSocket = ServerSocketQueue.front();
            ServerSocketQueue.pop();
            thread ServerRSThread = thread(ServerRS, ServerSocket);
            ServerRSThread.detach();
        }
        ServerQueueLock.exchange(false, std::memory_order_release);
    }
}
void ClientConnect()
{
    while (1)
    {
        while (ClientQueueLock.exchange(true, std::memory_order_acquire))
            ;
        if (ClientSocketQueue.size() > 0)
        {
            SOCKET ClientSocket = ClientSocketQueue.front();
            ClientSocketQueue.pop();
            ClientRSThreadArry.push_back(thread(ClientRS, ClientSocket));
        }
        ClientQueueLock.exchange(false, std::memory_order_release);
    }
}
void saveData()
{
    ofstream outss;
    outss.open("clientData.txt", ios::out);
    for (int i = 1; i <= DataSaveArry.size(); i++)
    {
        outss << DataSaveArry[i - 1].ClientWanIp
              << " " << DataSaveArry[i - 1].ClientLanIp
              << " " << DataSaveArry[i - 1].ClientConnectPort
              << " " << to_string(DataSaveArry[i - 1].OnlineTime)
              << " " << to_string(DataSaveArry[i - 1].OfflineTime)
              << endl;
    }
    outss.close();
}
void dataSave()
{
    while (1)
    {
        if (dataIsChange)
        {
            saveData();
        }
        Sleep(1000);
    }
}
void passData()
{

    while (1)
    {
        if (!PassDataInit)
        {
            while (ClientMapLock.exchange(true, std::memory_order_acquire))
                ;
            for (int i = 1; i <= ClientMap.size(); i++)
            {
                if (ClientMap[i - 1].state != ClientSocketFlagStruct::states::Use && ClientMap[i - 1].Offline != 0 && (ClientMap[i - 1].Offline - ClientMap[i - 1].Online) >= 3600)
                {
                    delForId(i);
                }
                else
                    DataSaveArry.push_back(ClientMap[i - 1]);
            }
            ClientMapLock.exchange(false, std::memory_order_release);
            Sleep(1000);
            PassDataInit = true;
            continue;
        }
        while (ClientMapLock.exchange(true, std::memory_order_acquire))
            ;
        for (int i = 1; i <= ClientMap.size(); i++)
        {
            if (ClientMap[i - 1].state != ClientSocketFlagStruct::states::Use && ClientMap[i - 1].Offline != 0 && (ClientMap[i - 1].Offline - ClientMap[i - 1].Online) >= 3600)
            {
                delForId(i);
            }
            else
            {
                vector<ClientSocketFlagStruct>::iterator itr = find(DataSaveArry.begin(), DataSaveArry.end(), ClientMap[i - 1]);
                if (itr != DataSaveArry.end())
                {
                    DataSaveArry[distance(DataSaveArry.begin(), itr)] = ClientMap[i - 1];
                }
            }
        }
        ClientMapLock.exchange(false, std::memory_order_release);
    }
}
void loadData()
{
    ifstream inss, inPassword;
    inPassword.open("password.data", ios::in);
    if (!inPassword)
    {
        ofstream out;
        out.open("password.data", ios::out);
        out.close();
        inPassword.close();
        exit(0);
    }
    else
    {
        inPassword >> password;
        cout << "Load Password:" << password << endl;
        password = "08d0a4449012a585c411c84202e64a73";
    }
    inPassword.close();
    inss.open("clientData.data", ios::in);
    if (!inss)
    {
        ofstream out;
        out.open("clientData.data", ios::out);
        out.close();
        inss.close();
        return;
    }
    string wanip, lanip, port, OnlineTime, OfflineTime;
    while (inss >> wanip >> lanip >> port >> OnlineTime >> OfflineTime)
    {
        joinClient(wanip, lanip, port, NULL, stoi(OnlineTime), stoi(OfflineTime));
    }
    inss.close();
}
int main(int argc, char **argv)
{
    system("chcp 65001>nul");
    SetConsoleCtrlHandler(HandlerRoutine, TRUE);
    loadData();
    cout << "loadData OK\n";
    cout << "password:" << password << endl;
    initServer(ListenSocket, service, 2060);

    thread ClientConnectThread = thread(ClientConnect);
    thread ServerConnectThread = thread(ServerConnect);
    thread dataSaveThread = thread(dataSave);
    thread PassDataThread = thread(passData);
    cout << "server start";
    while (true)
    {
        string buf;
        SOCKET aptSocket;
        sockaddr_in aptsocketAddr = {0};
        int len = sizeof(aptsocketAddr);
        aptSocket = accept(ListenSocket, (SOCKADDR *)&aptsocketAddr, &len);
        if (aptSocket != INVALID_SOCKET)
        {
            receive_message(aptSocket, buf);
            if (strcmp(buf.c_str(), "Client") == 0)
            {
                send_message(aptSocket, "Recv");
                while (ClientQueueLock.exchange(true, std::memory_order_acquire))
                    ;
                ClientSocketQueue.push(aptSocket);
                ClientQueueLock.exchange(false, std::memory_order_release);
                // aptSocket = INVALID_SOCKET;
            }
            else if (strcmp(buf.c_str(), "Server") == 0)
            {
                cout << "One New\n";
                send_message(aptSocket, "Recv");
                while (ServerQueueLock.exchange(true, std::memory_order_acquire))
                    ;
                ServerSocketQueue.push(aptSocket);
                ServerQueueLock.exchange(false, std::memory_order_release);
                // aptSocket = INVALID_SOCKET;
            }
            else if (ServerSEIDMap[buf].isSEIDExit)
            {
                ServerSEIDMap[buf].getServerHealthySocketLock();
                ServerSEIDMap[buf].getOtherValueLock();
                ServerSEIDMap[buf].socketH = aptSocket;
                ServerSEIDMap[buf].isSocketExit = true;
                ServerSEIDMap[buf].releaseOtherValueLock();
                ServerSEIDMap[buf].releaseServerHealthySocketLock();
            }
            else if (ClientSEIDMap[buf].isSEIDExit)
            {

                ClientSEIDMap[buf].socketH = aptSocket;
                ClientSEIDMap[buf].isSocketExit = true;
            }
        }
    }
    return 0;
}