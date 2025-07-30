// client_server.cpp : 定义 DLL 的导出函数。
//
#include "pch.h"
#include "framework.h"
#include "client_server.h"
#include "..\include\globalDefine.h"
#include "..\include\serverStruct.h"
#include "..\include\fliter.h"
namespace plugin_serverNamespace
{
#define initClientSocket_
	static void healthyBeat(SOCKET& sock, bool* stopSign)
	{
#ifdef DEBUG
		while (!(*stopSign))
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
#else
		auto prlog = (*g_log).getFunLog("healthyBeat");
		int timeout = 3000;
		setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
		setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));
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
#endif
		* stopSign = true;
		closesocket(sock);
		return;
	}
};
class server : public PluginNamespace::pluginBase
{
public:
	bool runFun(PluginNamespace::Info& info) override
	{
		if (info.customize_data.size() == 3 &&
			(*((std::shared_ptr<std::string> &)(info.customize_data.back()))).find("false") != std::string::npos)
		{
			(*((std::shared_ptr<std::string> &)(info.customize_data.back()))) = "true";
			return true;
		}

		info.customize_data.pop_back(); // pop server mode flag
		show = false;

		std::shared_ptr<programPluginInfoStruct>& funInfo = (std::shared_ptr<programPluginInfoStruct> &)(info.customize_data[0]);
		std::shared_ptr<PluginInfoStruct>& pluginInfo = (std::shared_ptr<PluginInfoStruct> &)(info.customize_data[1]);
		std::shared_ptr<Log>& log = pluginInfo->log;
		std::shared_ptr<std::string>& connectIp = funInfo->connectIp;
		int connectPort = funInfo->connectPort;
		pluginManager = std::make_shared<PluginNamespace::PluginManager>();
		PluginNamespace::loadPlugin(*pluginManager, std::string(".\\server_plugin\\"));

		std::shared_ptr<SOCKET> mainConnectSocket = funInfo->mainConnectSocket;
		initClientSocket(g_wsaData, *mainConnectSocket, g_sockaddr, *connectIp, connectPort);

		std::shared_ptr<SOCKET> healthySocket = funInfo->healthySocket;

		std::string helloMsg = "S" + getLanIp() + "W" + (*((std::shared_ptr<std::string> &)(info.customize_data.back())));

		info.customize_data.pop_back(); // pop server msg

		while (connect(*mainConnectSocket, (sockaddr*)&g_sockaddr, sizeof(g_sockaddr)))
			;
		send(*mainConnectSocket, helloMsg);
		recv(*mainConnectSocket, helloMsg);

		if (helloMsg == "OK")
		{
			recv(*mainConnectSocket, helloMsg);

			initClientSocket(g_wsaData, *healthySocket, g_sockaddr, *connectIp, connectPort);
			while (connect(*healthySocket, (sockaddr*)&g_sockaddr, sizeof(g_sockaddr)))
				;

			send(*healthySocket, 'H' + helloMsg);
			recv(*healthySocket, helloMsg);

			if (helloMsg == "OK")
			{
				std::thread healthyBeatThread = std::thread(plugin_serverNamespace::healthyBeat, std::ref(*healthySocket), &stopSign);
				std::string buf;

				auto temp = recvAndMatchPluginList(*mainConnectSocket);
				for (auto& i : temp)
					std::cout << 1 << i << std::endl;
				auto localPluginList = pluginManager->getLocalPluginName();
				std::vector<std::string> pluginList;
				for (auto& i : temp)
					pluginList.push_back(i);
				for (auto& i : localPluginList)
					pluginList.push_back(i);
				while (!this->stopSign)
				{
					system("cls");
					int index = 0, inputIndex = 0;
					for (auto& it : pluginList)
						std::cout << "[ " << index++ << " ] pluginName:" << it << std::endl;
					std::cout << "Tips: flash plugin list input -1,exit server input -2" << std::endl;
					std::cout << "input plugin index:";
					inputIndex = 0;
					std::cin >> inputIndex;

					if (inputIndex == -1)
					{
						send(*mainConnectSocket, "\r\nflash\r\n");
						temp = recvAndMatchPluginList(*mainConnectSocket);
						continue;
					}
					else if (inputIndex == -2)
					{
						send(*mainConnectSocket, "\r\nexit\r\n");
						stopSign = true;
						continue;
					}
					else if (inputIndex >= 0 && inputIndex < pluginList.size())
					{
						if (!(*pluginManager).isFunLocal(pluginList[inputIndex]))
						{
							send(*mainConnectSocket, pluginList[inputIndex]);
							recv(*mainConnectSocket, buf);
							if (buf == "success")
							{
								send(*mainConnectSocket, "NULL"); // plugin arguments is NULL

								Info info;
								std::shared_ptr<PluginInfoStruct> PInfo(new PluginInfoStruct(pluginManager, g_log));
								PInfo->pluginManager = pluginManager;
								PInfo->log = log;

								auto programInfo = std::make_shared<programPluginInfoStruct>();
								programInfo->mainConnectSocket = mainConnectSocket;
								programInfo->healthySocket = healthySocket;
								programInfo->connectIp = connectIp;
								programInfo->connectPort = connectPort;

								info.customize_data.push_back((std::shared_ptr<void>)programInfo);
								info.customize_data.push_back((std::shared_ptr<void>)PInfo);
								(*pluginManager).runFun(pluginList[inputIndex], info);
							}
							else
							{
								std::cout << "Don't found plugin or run failed";
								system("pause");
							}
						}
						else
						{
							Info info;
							std::shared_ptr<PluginInfoStruct> PInfo(new PluginInfoStruct(pluginManager, g_log));
							info.customize_data.push_back(std::make_shared<programPluginInfoStruct>());
							info.customize_data.push_back((std::shared_ptr<void>)PInfo);

							std::shared_ptr<programPluginInfoStruct>& funInfo = (std::shared_ptr<programPluginInfoStruct> &)(info.customize_data[0]);
							std::shared_ptr<PluginInfoStruct>& pluginInfo = (std::shared_ptr<PluginInfoStruct> &)(info.customize_data[1]);
							std::shared_ptr<SOCKET> sock = funInfo->mainConnectSocket;
							(*pluginManager).runFun(pluginList[inputIndex], info);
						}
					}
					else
						send(*mainConnectSocket, "next");
				}
				healthyBeatThread.join();
			}
		}
		WSACleanup();
		show = true;
		return true;
	}
	server()
	{
		this->used = true;
		this->show = true;
		this->pluginName = "server";
		this->version = "1.0.0";
		this->author = "x3fang";
		this->local = true;
		this->g_sockaddr = { 0 };
		this->g_wsaData = { 0 };

	}
	bool getServerStatus() const
	{
		return stopSign;
	}

private:
	std::shared_ptr<PluginNamespace::PluginManager> pluginManager;
	bool stopSign = false;
	WSADATA g_wsaData;
	sockaddr_in g_sockaddr;
	std::string getLanIp()
	{
		SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
		HANDLE hReadPipe, hWritePipe;
		if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0))
			return "127.0.0.1";

		// 合并命令：先修改代码页为UTF-8，再执行ipconfig
		const char* cmd = "cmd.exe /c \"chcp 65001 >nul && ipconfig\"";

		STARTUPINFOA si = { sizeof(STARTUPINFOA) };
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
		char buffer[4096] = { 0 };
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
	std::vector<std::string> recvAndMatchPluginList(SOCKET& sock)
	{
		std::string buf;
		std::vector<std::string> pluginList;
		while (buf != "end")
		{
			recv(sock, buf);
			if (buf != "end" && (*pluginManager).findFun(buf))
				pluginList.push_back(buf);
		}
		return pluginList;
	}
};
static server Plugin_serverClient;
extern "C"
{
	EXPORT bool registerFun(PluginNamespace::PluginManager& pluginManager)
	{
		return pluginManager.registerFun(&Plugin_serverClient).first;
	}
}