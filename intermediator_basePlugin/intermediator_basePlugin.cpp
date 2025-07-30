// intermediator_basePlugin.cpp : 定义 DLL 的导出函数。
//

#include "pch.h"
#include "framework.h"
#include "intermediator_basePlugin.h"
#include "..\include\globalDefine.h"
#include "..\include\intermediatorStruct.h"
#include "..\include\fliter.h"
#include <iostream>
#include <sstream>
class find : public PluginNamespace::pluginBase
{
public:
	bool runFun(PluginNamespace::Info& info) override
	{
		std::shared_ptr<programPluginInfoStruct>& funInfo = (std::shared_ptr<programPluginInfoStruct> &)(info.customize_data[0]);
		std::shared_ptr<PluginInfoStruct>& pluginInfo = (std::shared_ptr<PluginInfoStruct> &)(info.customize_data[1]);
		std::shared_ptr<SOCKET> sock = funInfo->mainConnectSocket;
		auto clientInfo = pluginInfo->ClientInfo;
		std::string data;

		while (true)
		{
			int res = recv(*sock, data);
			if (res == SUCCESS_STATUS)
			{
				if (data == "\r\nexit\r\n")
					break;
				std::stringstream ss(data);
				std::vector<std::string> sendData;
				sendData.push_back("0");
				for (auto& it : (*clientInfo))
					if (it.second->lanIp == data)
						sendData.push_back(it.second->wanIp + " " +
							it.second->lanIp + " " +
							std::to_string(it.second->systemKind) + " " +
							it.second->SEID + "\r" +
							it.second->commit);
				while (sendData.back() != "0")
				{
					if (send(*sock, sendData.back()) == SUCCESS_STATUS)
						sendData.pop_back();
				}
				send(*sock, "end");
			}
		}

		return false;
	}
	find()
	{
		this->used = true;
		this->show = false;
		this->pluginName = "findClient";
		this->version = "1.0.0";
		this->author = "x3fang";
	}
};
class delClient : public PluginNamespace::pluginBase
{
public:
	bool runFun(PluginNamespace::Info& info) override
	{
		std::shared_ptr<programPluginInfoStruct>& funInfo = (std::shared_ptr<programPluginInfoStruct> &)(info.customize_data[0]);
		std::shared_ptr<PluginInfoStruct>& pluginInfo = (std::shared_ptr<PluginInfoStruct> &)(info.customize_data[1]);
		std::shared_ptr<SOCKET> sock = funInfo->mainConnectSocket;
		auto clientInfo = pluginInfo->ClientInfo;
		std::string data;
		if (recv(*sock, data) == SUCCESS_STATUS) // recv client SEID
		{
			std::stringstream ss(data);
			if (send(*sock,
				(((std::shared_ptr<PluginInfoStruct> &)(info.customize_data[1]))
					->delClient(data.substr(1))) // run delClient
				? "succeed"
				: "failed") == SUCCESS_STATUS)
				return true;
			else
				return false;
		}
		return false;
	}
	delClient()
	{
		this->used = true;
		this->show = false;
		this->pluginName = "delClient";
		this->version = "1.0.0";
		this->author = "x3fang";
	}
};
class showClient : public PluginNamespace::pluginBase
{
public:
	bool runFun(PluginNamespace::Info& info) override
	{
		std::shared_ptr<programPluginInfoStruct>& funInfo = (std::shared_ptr<programPluginInfoStruct> &)(info.customize_data[0]);
		std::shared_ptr<PluginInfoStruct>& pluginInfo = (std::shared_ptr<PluginInfoStruct> &)(info.customize_data[1]);
		std::shared_ptr<SOCKET> sock = funInfo->mainConnectSocket;
		auto clientInfo = pluginInfo->ClientInfo;

		auto funlog = pluginInfo->log->getFunLog("showClientRunFun");
		std::string sendMsg;
		std::shared_ptr<Fliter> fliter = nullptr;
		if (info.customize_data.size() >= 3)
		{
			fliter = std::static_pointer_cast<Fliter>(info.customize_data[2]);
			fliter->addRuleType("use", EQUAL | NOT_EQUAL);
			fliter->addRuleType("wanIp", EQUAL | NOT_EQUAL);
			fliter->addRuleType("lanIp", EQUAL | NOT_EQUAL);
			fliter->addRuleType("commit", EQUAL | NOT_EQUAL);
			fliter->addRuleType("systemKind", EQUAL | NOT_EQUAL);
		}
		if (!fliter)
		{
			fliter = std::make_shared<Fliter>();
			fliter->addRuleType("use", EQUAL | NOT_EQUAL);
			fliter->addRuleType("wanIp", EQUAL | NOT_EQUAL);
			fliter->addRuleType("lanIp", EQUAL | NOT_EQUAL);
			fliter->addRuleType("commit", EQUAL | NOT_EQUAL);
			fliter->addRuleType("systemKind", EQUAL | NOT_EQUAL);
		}
		if (info.customize_data.size() >= 4)
		{
			std::vector<std::string> data_splitBySpace = info.getArguments_splitBySpaceList();
			std::vector<std::string> fliterTypeNameArgList;
			std::vector<std::vector<std::string>> fliterValueArgList;
			std::vector<std::vector<std::string>> fliterConditionArgList;
			int index = 0;

			for (std::string name = data_splitBySpace.front(), value, condition; !data_splitBySpace.empty();)
			{

				data_splitBySpace.erase(data_splitBySpace.begin());
				while (true)
				{
					if (data_splitBySpace.size() >= 3)
					{
						int delList = 0;
						auto it = data_splitBySpace.begin();

						value = *it;
						delList++;

						condition = *(++it);
						delList++;

						while (*(it + 1) != "|")
						{
							condition += *(++it);
							delList++;
						}
						fliterValueArgList[index].push_back(value);
						fliterConditionArgList[index].push_back(condition);
						for (int i = 0; i < delList; i++)
						{
							data_splitBySpace.erase(data_splitBySpace.begin());
						}
					}
					else
						continue;
				}
				index++;
				fliterTypeNameArgList.push_back(name);
				name = data_splitBySpace.front();
			}
			std::vector<std::string> fliterRuleType = fliter->getAllRuleType();

			index = 0;
			for (auto& ruleType : fliterRuleType)
			{
				if (std::find(fliterTypeNameArgList.begin(), fliterTypeNameArgList.end(), ruleType) != fliterTypeNameArgList.end())
				{
					for (int i = 0; i < fliterTypeNameArgList.size(); i++)
						fliter->addRule(ruleType, fliterValueArgList[index][i], FliterOption[fliterConditionArgList[index][i]]);
				}
				++index;
			}
		}
		return sendOnce(sock, *(pluginInfo->ClientInfo), *fliter);
	}
	showClient()
	{
		this->used = true;
		this->show = false;
		this->pluginName = "showClient";
		this->version = "1.0.0";
		this->author = "x3fang";
	}

private:
	bool sendOnce(std::shared_ptr<SOCKET>& sock, std::map<std::string, std::shared_ptr<IndividualInfoStruct>>& infoMap, Fliter& fliter)
	{
		for (auto it = infoMap.begin(); it != infoMap.end(); it++)
		{
			auto data = it->second.get();
			if ((fliter.matchRule("wanIp", (*data).wanIp) &&
				fliter.matchRule("lanIp", (*data).lanIp) &&
				fliter.matchRule("systemKind", std::to_string((*data).systemKind)) &&
				fliter.matchRule("commit", (*data).commit) &&
				fliter.matchRule("use", std::to_string(((*data).use ? 1 : 0)))))
			{
				std::string sendMsg((*data).wanIp + " " +
					(*data).lanIp + " " +
					std::to_string((*data).systemKind) + " " +
					(*data).SEID + "\r\n" +
					(*data).commit);
				int res = send(*sock, sendMsg);
				if (res != SUCCESS_STATUS)
				{
					send(*sock, "end");
					return false;
				}
			}
		}
		send(*sock, "end");
		return true;
	}
};
class connectClient : public PluginNamespace::pluginBase
{
public:
	bool runFun(PluginNamespace::Info& info) override
	{
		std::shared_ptr<programPluginInfoStruct>& funInfo = (std::shared_ptr<programPluginInfoStruct> &)(info.customize_data[0]);
		std::shared_ptr<PluginInfoStruct>& pluginInfo = (std::shared_ptr<PluginInfoStruct> &)(info.customize_data[1]);
		std::shared_ptr<SOCKET> serverSock = funInfo->mainConnectSocket;
		auto clientInfo = pluginInfo->ClientInfo;
		bool status = true;
		auto funlog = pluginInfo->log->getFunLog("connectClientRunFun");

		std::string data;

		Fliter fliter;
		fliter.addRuleType("use", EQUAL | NOT_EQUAL);
		fliter.addRule("use", "0", EQUAL);
		auto showClientInfo = info;
		showClientInfo.customize_data.push_back((std::shared_ptr<void>)(std::make_shared<Fliter>(fliter)));

		while (true)
		{
			pluginInfo->pluginManager->runFun("showClient", showClientInfo);
			int res = recv(*serverSock, data);
			if (res == SUCCESS_STATUS)
			{
				if (data == "\r\nnext\r\n")
					continue;
				else if (data == "\r\nexit\r\n")
					break;
				else if (pluginInfo->ClientInfo->find(data) != pluginInfo->ClientInfo->end())
				{
					std::string clientSEID = data;
					pluginInfo->ClientInfo->at(data)->waitLock();
					auto clientSock = pluginInfo->ClientInfo->at(data)->commSocket;
					sendPluginList(*pluginInfo->pluginManager, *clientSock);
					send(*clientSock, pluginName);
					recv(*clientSock, data);

					send(*serverSock, data);
					if (data == "ok")
					{
						recv(*clientSock, data);
						send(*serverSock, data);
						while (true)
						{
							data.clear();
							recv(*serverSock, data);
							send(*clientSock, data);
							recv(*clientSock, data);
							funlog->writeln(data);
							send(*serverSock, data);
							if (data == "\r\n[exit]\r\n")
								break;
						}
					}
					data.clear();
					pluginInfo->ClientInfo->at(clientSEID)->unLock();
				}
				else
					send(*serverSock, "failed");
			}
			else
			{
				status = false;
				break;
			}
		}
		return status;
	}
	connectClient()
	{
		this->used = true;
		this->show = true;
		this->pluginName = "connectClient";
		this->version = "1.0.0";
		this->author = "x3fang";
	}

private:
	void sendPluginList(PluginNamespace::PluginManager& pluginManager, SOCKET& sock)
	{
		auto pluginList = pluginManager.getAllPluginName();
		for (auto& pluginName : pluginList)
			send(sock, pluginName);
		send(sock, "end");
		return;
	}
};
static find Plugin_find;
static delClient Plugin_delClient;
static showClient Plugin_showClient;
static connectClient Plugin_connectClient;
extern "C"
{
	EXPORT bool registerFun(PluginNamespace::PluginManager& pluginManager)
	{
		return pluginManager.registerFun(&Plugin_find).first &&
			pluginManager.registerFun(&Plugin_delClient).first &&
			pluginManager.registerFun(&Plugin_showClient).first &&
			pluginManager.registerFun(&Plugin_connectClient).first;
	}
}