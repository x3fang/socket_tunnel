#pragma once

#include <arpa/inet.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define EXPORT __attribute__((visibility("default")))
#define SOCKET int

//#define /*DEBUG*/ // if define it,healthy Beat won't work
#include <algorithm>
#include <cstring>
#include <string>
#include <map>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <iostream>
#include "sign.h"
#include "log.h"

using namespace logNameSpace;
std::shared_ptr<std::string> connectIp(std::make_shared<std::string>());
int connectPort;
std::shared_ptr<SOCKET> mainConnectSocket(std::make_shared<SOCKET>());
std::shared_ptr<SOCKET> healthySocket(std::make_shared<SOCKET>());
std::shared_ptr<Log> g_log(std::make_shared<Log>());
std::atomic<bool> isUse(false);
struct infoLock
{
private:
	std::mutex valueLock;
	std::condition_variable valueChange;
	std::unique_lock<std::mutex> lock;
	bool use = false;
	bool del = false;

public:
	void setDel(bool del)
	{
		this->del = del;
	}
	int waitLock()
	{
		if (del)
			return WILL_DEL_STATUS;
		lock = std::unique_lock<std::mutex>(valueLock);
		bool& inUse = use;
		valueChange.wait(lock, [&inUse]
			{ return !inUse; });

		use = true;
		return SUCCESS_STATUS;
	}
	void unLock()
	{
		if (!lock.owns_lock())
			return;
		use = false;
		lock.unlock();
		valueChange.notify_one();
	}
};
int send(SOCKET& sock, const std::string& data)
{
	std::string temp(std::to_string(data.length()) + "\r" + data);
	int res = send(sock, temp.c_str(), static_cast<int>(temp.size()), 0);
	if (res <= 0)
		return errno;
	return SUCCESS_STATUS;
}
int recv(SOCKET& sock, std::string& data)
{
	data.clear();
	try
	{
		std::string resData;
		char buf[1028] = { 0 };
		int recvDataLength = 0;
		while (true)
		{
			int res = recv(sock, buf, 1, 0);
			if (res <= 0)
				return errno;
			if (strlen(buf) > 0 && buf[0] == '\r')
			{
				if (recvDataLength != 0)
				{
					while (recvDataLength > 0)
					{
						if (recvDataLength > 1024)
							recv(sock, buf, 1024, 0);
						else if (recvDataLength >= 0)
							recv(sock, buf, recvDataLength, 0);
						recvDataLength -= 1024;
						data += buf;
						memset(buf, 0, sizeof(buf));
					}
					return SUCCESS_STATUS;
				}
				return -1;
			}
			recvDataLength *= 10;
			recvDataLength += atoi(buf);
		}
	}
	catch (const std::exception& e)
	{
		throw e.what();
	}
	return -1;
}
#include "Plugin.h"
#include "programPluginInfo.h"
std::shared_ptr<PluginNamespace::PluginManager> pluginManager(std::make_shared<PluginNamespace::PluginManager>());
using PluginNamespace::Info;