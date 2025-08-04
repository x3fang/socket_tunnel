#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include "../include/WindowsFunc.h"
#else
#include "../include/Ubuntu.h"
#endif
#include "../../include/log.h"
#include "../../include/MD5.h"
#include "../../include/intermediatorStruct.h"
#include <random>
bool ServerStopFlag = false;
sockaddr_in g_sockaddr;
std::thread healthyBeatThread;

std::shared_ptr<std::map<std::string, std::shared_ptr<IndividualInfoStruct>>> ServerInfo(std::make_shared<std::map<std::string, std::shared_ptr<IndividualInfoStruct>>>());

std::shared_ptr<std::map<std::string, std::shared_ptr<IndividualInfoStruct>>> ClientInfo(std::make_shared<std::map<std::string, std::shared_ptr<IndividualInfoStruct>>>());

std::vector<healthyBeatInfoStruct> healthyBeatSOCKETList;
std::vector<std::thread> serverThreadArry;
const std::string createSEID(const std::string& temp)
{
	MD5::MD5 md5;
	return md5.encode(temp);
}
int registerCOS(SOCKET socket,
	const std::string& wanIp,
	const std::string& lanIp,
	int systemKind,
	const std::string& commit,
	std::string& SEID_res,
	std::map<std::string, std::shared_ptr<IndividualInfoStruct>>& infoMap)
{
	static auto prlog = (*g_log).getFunLog("registerCOS");
	std::string SEID = createSEID(lanIp + wanIp + std::to_string(systemKind) + commit);
	prlog->writeln("SEID:" + SEID);
	if (infoMap.find(SEID) == infoMap.end())
	{
		infoMap[SEID] = std::make_shared<IndividualInfoStruct>(SEID, wanIp, lanIp, systemKind, commit, socket, -1);
		SEID_res = SEID;
		return SUCCESS_STATUS;
	}
	return FAIL_STATUS;
}
inline int registerClient(SOCKET socket, const std::string& wanIp, const std::string& lanIp, int systemKind, const std::string& commit, std::string& SEID_res)
{
	return registerCOS(socket, wanIp, lanIp, systemKind, commit, SEID_res, *ClientInfo);
}
inline int registerServer(SOCKET socket, const std::string& wanIp, const std::string& lanIp, int systemKind, const std::string& commit, std::string& SEID_res)
{
	return registerCOS(socket, wanIp, lanIp, systemKind, commit, SEID_res, *ServerInfo);
}
int del(const std::string& SEID, std::map<std::string, std::shared_ptr<IndividualInfoStruct>>* infoMap)
{
	if ((*infoMap).find(SEID) != (*infoMap).end())
	{
		int status = (*(*infoMap)[SEID]).waitLock();
		if (status == OTHER_THREAD_IS_PROCESSING_STATUS)
			return OTHER_THREAD_IS_PROCESSING_STATUS;
		else if (status != SUCCESS_STATUS)
			return status;
		(*(*infoMap)[SEID]).del = true;
		if (*(*infoMap)[SEID]->healthSocket > 0)
		{
			send(*(*infoMap)[SEID]->healthSocket, "del");
			closesocket(*(*infoMap)[SEID]->healthSocket);
			*(*infoMap)[SEID]->healthSocket = -1;
			healthyBeatSOCKETList.erase(std::find(healthyBeatSOCKETList.begin(), healthyBeatSOCKETList.end(), SEID));
		}
		send(*(*infoMap)[SEID]->commSocket, "end");
		closesocket(*(*infoMap)[SEID]->commSocket);
		*(*infoMap)[SEID]->commSocket = -1;
		(*(*infoMap)[SEID]).unLock();
		(*infoMap).erase(SEID);
		return SUCCESS_STATUS;
	}
	return FAIL_STATUS;
}
inline bool find(const std::string& SEID, std::shared_ptr<std::map<std::string, std::shared_ptr<IndividualInfoStruct>>> infoMap)
{
	return (*infoMap).find(SEID) != (*infoMap).end();
}
int delClient(const std::string& SEID)
{
	static auto prlog = (*g_log).getFunLog("delClient");
	prlog->writeln("Del client's SEID:" + SEID);
	return del(SEID, &*ClientInfo);
}
int delServer(const std::string& SEID)
{
	static auto prlog = (*g_log).getFunLog("delServer");
	prlog->writeln("Del server's SEID:" + SEID);
	return del(SEID, &*ServerInfo);
}

bool IsValidIP(const char* ip)
{
	in_addr addr;
	return inet_pton(AF_INET, ip, &addr) == 1;
}
int arrangeRegister(const std::string& buf, std::string& lanIp_res, int& systemKind_res, std::string& commit_res)
{
	static auto prlog = (*g_log).getFunLog("arrangeRegister");
	int LSS = static_cast<int>(buf.find_first_not_of("0123456789.", 1)); // lanIp and system kinds separator
	std::string lanIP = buf.substr(1, LSS - 1);
	if (!lanIP.empty() &&
		std::count(lanIP.begin(), lanIP.end(), '.') == 3 &&
		IsValidIP(lanIP.c_str()))
	{
		lanIp_res = lanIP;
		std::string commit;
		int systemKind = -1;
		if (LSS != std::string::npos)
			systemKind = (buf[LSS] == 'W' ? 0 : 1);
		commit = buf.substr(LSS + 1);
		commit_res = commit;
		systemKind_res = systemKind;
		return SUCCESS_STATUS;
	}
	else
	{
		prlog->writeln("failed to arrange data");
		prlog->writeln("data: " + buf);
	}
	return FAIL_STATUS;
}
void sendPluginList(PluginNamespace::PluginManager& pluginManager, SOCKET& sock)
{
	auto pluginList = pluginManager.getAllPluginName();
	for (auto& pluginName : pluginList)
		send(sock, pluginName);
	send(sock, "end");
	return;
}
void healthyBeat()
{
#ifdef DEBUG
	while (!stopFlag)
		;
#else
	static auto prlog = (*g_log).getFunLog("healthyBeat");
	std::default_random_engine random_engine;
	random_engine.seed(time(0));
	std::string buf;
	while (!ServerStopFlag)
	{
		if (!healthyBeatSOCKETList.empty())
		{
			auto radom = random_engine();
			for (auto& healthyInfo : healthyBeatSOCKETList)
			{
				int timeout = 3000;
				setsockopt(healthyInfo.healthSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
				setsockopt(healthyInfo.healthSocket, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));
				int res1 = send(healthyInfo.healthSocket, std::to_string(radom));
				int res2 = recv(healthyInfo.healthSocket, buf);
				if (buf != std::to_string(radom) || res1 != SUCCESS_STATUS || res2 != SUCCESS_STATUS)
				{
					prlog->writeln(std::string("one ") + std::string((healthyInfo.server ? "server" : "client")) +
						" disconnect\n" + "SEID:" +
						healthyInfo.SEID);
				}
				buf.clear();
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
#endif
}
void serverThread(const std::string SEID)
{
	auto prlog = (*g_log).getFunLog(("Server-SEID:" + SEID));
	auto info = (*ServerInfo)[SEID];
	prlog->writeln("waiting for server healthy socket connect");
	while (!(*info->healthSocket))
		;
	prlog->writeln("server healthy socket connect success");
	if (info->waitLock() != SUCCESS_STATUS)
		return;
	std::string buf;
	sendPluginList(*pluginManager, *info->commSocket);
	while (!ServerStopFlag)
	{
		int res = recv(*info->commSocket, buf);
		if (res == SUCCESS_STATUS)
		{
			if (buf.find("\r\nexit\r\n") != std::string::npos)
			{
				prlog->writeln("server exit");
				info->unLock();
				delServer(SEID);
				return;
			}
			else if (buf == "next")
				continue;
			else if (buf == "\r\nflash\r\n")
			{
				sendPluginList(*pluginManager, *info->commSocket);
				continue;
			}
			else if ((*pluginManager).findFun(buf))
			{
				prlog->writeln("plugin name:" + buf);
				std::shared_ptr<PluginInfoStruct> PInfo(new PluginInfoStruct);
				(*PInfo).log = ::g_log;
				(*PInfo).pluginManager = ::pluginManager;
				(*PInfo).ClientInfo = ::ClientInfo;
				(*PInfo).ServerInfo = ::ServerInfo;
				(*PInfo).find = ::find;
				(*PInfo).delClient = ::delClient;

				Info runFunInfo;

				auto pluginInfoTemp = std::make_shared<programPluginInfoStruct>();
				(*pluginInfoTemp).mainConnectSocket = info->commSocket;
				(*pluginInfoTemp).healthySocket = info->healthSocket;
				(*pluginInfoTemp).SEID = info->SEID;

				runFunInfo.customize_data.push_back(pluginInfoTemp);
				runFunInfo.customize_data.push_back((std::shared_ptr<void>)PInfo);
				send(*info->commSocket, "success");

				std::string argRecvBuf, argRecvBuf_toupper;
				recv(*info->commSocket, argRecvBuf);
				for (char c : argRecvBuf)
					argRecvBuf_toupper += std::toupper(c);
				if (argRecvBuf_toupper != "NULL")
				{
					runFunInfo.setArguments(argRecvBuf);
				}

				if ((*pluginManager).runFun(buf, runFunInfo))
					send(*info->commSocket, "success");
				else
				{
					continue;
					send(*info->commSocket, "failed");
					prlog->writeln("runFun failed");
				}
			}
		}
		else
		{
			prlog->writeln("recv error:" + std::to_string(res) + " at:" + std::to_string(__LINE__));
			send(*info->commSocket, "failed");
		}
	}
	info->unLock();
	return;
}
int main()
{
	(*g_log).setDetailLevel(logNameSpace::Log::detailLevelEnum::debug);
	(*g_log).setName("intermediator");
	(*g_log).writeln("initializing program");

	(*g_log).writeln("init server");
	std::ifstream in("server.config");
	std::string connectIPInputBuff = "0.0.0.0";
	std::string connectPortInputBuff = "80";
	std::string pluginPathInputBuff = "./plugin/";
	bool loadInServerConfigFlag = false;
	int res = 0;
	if (in)
	{
		getline(in, connectIPInputBuff);
		getline(in, connectPortInputBuff);
		getline(in, pluginPathInputBuff);

		(*connectIp) = connectIPInputBuff;
		connectPort = std::stoi(connectPortInputBuff);
#if windowsSystem
		res = initServer(*mainConnectSocket, g_wsaData, g_sockaddr, std::stoi(connectPortInputBuff), connectIPInputBuff);
#else
		res = initServer(*mainConnectSocket, g_sockaddr, std::stoi(connectPortInputBuff), connectIPInputBuff);
#endif
		if (res == SUCCESS_STATUS)
			loadInServerConfigFlag = true;

		if (pluginPathInputBuff != "")
		{
			(*g_log).writeln("load plugin");
			res = PluginNamespace::loadPlugin((*pluginManager), pluginPathInputBuff, pluginExtsion);
			(*g_log).writeln("load" + std::to_string(res) + "plugin");
		}
		else
		{
			(*g_log).writeln("load plugin");
			res = PluginNamespace::loadPlugin((*pluginManager), "./plugin/", pluginExtsion);
			(*g_log).writeln("load" + std::to_string(res) + "plugin");
		}
	}
	if (!loadInServerConfigFlag)
	{
		do
		{
			std::cout << "input listen ip(input 0 chose 0.0.0.0,input -1 to exit):";
			std::cin >> connectIPInputBuff;
			if (connectIPInputBuff == "-1")
				return 0;
			std::cout << "input listen port:";
			std::cin >> connectPortInputBuff;
#if windowsSystem
			int res = initServer(*mainConnectSocket, g_wsaData, g_sockaddr, std::stoi(connectPortInputBuff), connectIPInputBuff);
#else
			int res = initServer(*mainConnectSocket, g_sockaddr, std::stoi(connectPortInputBuff), connectIPInputBuff);
#endif
			if (res == SUCCESS_STATUS)
			{
				(*connectIp) = (connectIPInputBuff == "0" ? "0.0.0.0" : connectIPInputBuff);
				connectPort = std::stoi(connectPortInputBuff);
				break;
			}
		} while (true);
	}

	if (res != SUCCESS_STATUS)
	{
		(*g_log).writeln("initServer error");
		return -1;
	}
	(*g_log).writeln("init server success");

	healthyBeatThread = std::thread(healthyBeat);
	(*g_log).writeln("created healthyBeat");
	std::cout << "server start" << std::endl
		<< "listening ip:" << *connectIp << std::endl
		<< "listening port:" << connectPort << std::endl;
	while (true)
	{
		std::string buf;
		SOCKET aptSocket;
		sockaddr_in aptsocketAddr = { 0 };
#if windowsSystem
		int len = sizeof(aptsocketAddr);
#else
		socklen_t len = sizeof(aptsocketAddr);
#endif

		(*g_log).writeln("waiting for connect");
		aptSocket = accept(*mainConnectSocket, (sockaddr*)&aptsocketAddr, &len);
		(*g_log).writeln("accept a connect");

		if (aptSocket > 0)
		{
			recv(aptSocket, buf);
			if (buf.length() <= 1)
			{
				(*g_log).writeln("invalid connect");
				closesocket(aptSocket);
				continue;
			}
#if windowsSystem
			CHAR ip_str_wide[INET_ADDRSTRLEN];
			InetNtopA(AF_INET, &aptsocketAddr.sin_addr, ip_str_wide, INET_ADDRSTRLEN);
			std::string wanIp = ip_str_wide;
#else
			std::string wanIp = inet_ntoa(aptsocketAddr.sin_addr);
#endif
			std::string lanIp, commit, SEID;
			int systemKind;
			switch (buf[0])
			{
			case 'C': // Client
				if (arrangeRegister(buf, lanIp, systemKind, commit) == SUCCESS_STATUS)
				{
					if (registerClient(aptSocket, wanIp, lanIp, systemKind, commit, SEID) == SUCCESS_STATUS)
					{
						(*g_log).writeln("\nRegister client,SEID:" + SEID +
							"\nwan ip:" + wanIp +
							"\nlan ip:" + lanIp +
							"\nsystemKind:" + (systemKind == 0 ? "Windows" : "Linux") +
							"\ncommit:" + commit);
						send(aptSocket, "OK");
						send(aptSocket, SEID);

						std::cout << "A client connected" << std::endl;
					}
					else
						goto failRegister;
				}
				else
					goto failRegister;
				break;
			case 'S': // Server
				if (arrangeRegister(buf, lanIp, systemKind, commit) == SUCCESS_STATUS)
				{
					if (registerServer(aptSocket, wanIp, lanIp, systemKind, commit, SEID) == SUCCESS_STATUS)
					{
						(*g_log).writeln("\nRegister server,SEID:" + SEID +
							"\nwan ip:" + wanIp +
							"\nlan ip:" + lanIp +
							"\nsystemKind:" + (systemKind == 0 ? "Windows" : "Linux") +
							"\ncommit:" + commit);
						send(aptSocket, "OK");
						send(aptSocket, SEID);
						serverThreadArry.push_back(std::thread(serverThread, SEID));

						std::cout << "A server connected" << std::endl;
					}
					else
						goto failRegister;
				}
				else
					goto failRegister;
				break;
			case 'H': // Healthy socket connect
				if ((*ClientInfo).find(buf.substr(1)) != (*ClientInfo).end())
				{
					(*g_log).writeln("Client healthy checking socket connect,SEID:" + buf.substr(1));
					*(*ClientInfo)[buf.substr(1)]->healthSocket = aptSocket;
					healthyBeatSOCKETList.push_back(healthyBeatInfoStruct(buf, aptSocket));
					send(aptSocket, "OK");
				}
				else if ((*ServerInfo).find(buf.substr(1)) != (*ServerInfo).end())
				{
					(*g_log).writeln("Server healthy checking socket connect,SEID:" + buf.substr(1));
					*(*ServerInfo)[buf.substr(1)]->healthSocket = aptSocket;
					healthyBeatSOCKETList.push_back(healthyBeatInfoStruct(buf, aptSocket, true));
					send(aptSocket, "OK");
				}
				else
					goto failRegister;
				std::cout << "Created a thread to check server or client health" << std::endl;
				break;
			failRegister:
				(*g_log).writeln("Register fail");
				send(aptSocket, "FAIL");
				closesocket(aptSocket);
			}
		}
	}
}