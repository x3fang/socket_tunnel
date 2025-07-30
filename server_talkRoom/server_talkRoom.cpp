// server_talkRoom.cpp : 定义 DLL 的导出函数。
//

#include "pch.h"
#include "framework.h"
#include "server_talkRoom.h"
#include "..\include\globalDefine.h"
#include "..\include\serverStruct.h"
#include "..\include\ConsoleChatUI.h"
#include "..\include\fliter.h"
#include <iostream>
#include <sstream>
#include <iomanip>
class talkRoom : public PluginNamespace::pluginBase
{
private:
	std::string selfName;
	std::string roomName;

public:
	bool runFun(PluginNamespace::Info& info) override
	{
		if (info.customize_data.empty())
			return false;

		std::shared_ptr<programPluginInfoStruct>& funInfo = (std::shared_ptr<programPluginInfoStruct> &)(info.customize_data[0]);
		std::shared_ptr<PluginInfoStruct>& pluginInfo = (std::shared_ptr<PluginInfoStruct> &)(info.customize_data[1]);
		sock = funInfo->mainConnectSocket;
		char choice;

		std::cout << "input your name:";
		std::cin >> selfName;

		std::cout << "create or join a talk room? (c/j):";
		std::cin >> choice;
		send(*sock, selfName);
		if (choice == 'c' || choice == 'C')
		{
			send(*sock, "server");
		}
		else if (choice == 'j' || choice == 'J')
		{
			send(*sock, "client");
			std::cout << "input the room name:";
			std::cin >> roomName;
			send(*sock, roomName);
		}
		talk();

		return true;
	}
	talkRoom()
	{
		this->used = true;
		this->show = true;
		this->pluginName = "talkRoom";
		this->version = "1.0.0";
		this->author = "x3fang";
	}

private:
	std::shared_ptr<SOCKET> sock;
	bool closeSignal = false;
	void talk();
	void recvMsg_client();
	std::shared_ptr<ConsoleChatUI> ui;
};
void talkRoom::recvMsg_client()
{
	std::string recvName;
	std::string recvBuf;
	std::string recvMsg;
	while (!closeSignal)
	{
		recv(*sock, recvName);
		if (recvName == "/exit")
		{
			break;
		}
		else if (recvName == "/server\\")
		{
			recv(*sock, recvBuf);
			if (recvBuf == "exit")
			{
				closeSignal = true;
				break;
			}
		}
		while (recvBuf != "end")
		{
			recv(*sock, recvBuf);
			if (recvBuf != "end")
				recvMsg += recvBuf + "\n";
			else
				break;
		}
		recvMsg.pop_back();
		ui->outputFunc(recvName + ":\n" + recvMsg);
		recvBuf.clear();
		recvName.clear();
		recvMsg.clear();
	}
	return;
}
void talkRoom::talk()
{
	int i = 0;
	system("cls");
	ui = std::make_shared<ConsoleChatUI>();
	ui->setPrompt("Format: [@User Name](only input '@',or not input,send to all user)(Shift+Enter)[Message]. Exit input \"/exit\">");
	ui->start();
	std::thread recv_showThread(&talkRoom::recvMsg_client, this);
	while (!closeSignal)
	{
		std::vector<std::string> input;
		input = ui->getUserInputByOnce();
		if (input.empty())
		{
			continue;
		}
		system("pause");
		if (input.front() == "/exit")
		{
			closeSignal = true;
			send(*sock, "/exit");
			break;
		}
		if (input.front() == "@")
		{
			input[0] = "NULL";
		}
		else if (input.front().find_first_of("@") == std::string::npos)
		{
			input.insert(input.begin(), "NULL");
		}
		send(*sock, input.front());
		input.erase(input.begin());
		for (auto& it : input)
		{
			send(*sock, it);
		}
		send(*sock, "end");
	}
	recv_showThread.join();
	ui->stop();
	std::cout << "talk thread exit";
	system("pause");
	return;
}
static talkRoom Plugin_talkRoom;
extern "C"
{
	EXPORT bool registerFun(PluginNamespace::PluginManager& pluginManager)
	{
		return pluginManager.registerFun(&Plugin_talkRoom).first;
	}
}