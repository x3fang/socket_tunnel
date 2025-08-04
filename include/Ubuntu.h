#pragma once
#include "../include/globalDefine.h"
#define closesocket close
int initServer(SOCKET& ListenSocket, sockaddr_in& sockAddr, const int& port, const std::string& connectIp = "0.0.0.0")
{
	int iResult = 0;
	ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ListenSocket <= 0)
	{
		int errorCode = 0;
		return errorCode;
	}

	sockAddr.sin_family = AF_INET;
	inet_pton(AF_INET, connectIp.c_str(), &sockAddr.sin_addr);
	sockAddr.sin_port = htons(port);
	iResult = bind(ListenSocket, (sockaddr*)&sockAddr, sizeof(sockAddr));
	if (iResult < 0)
	{
		int errorCode = 0;
		errorCode = errno;
		close(ListenSocket);
		return errorCode;
	}
	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult < 0)
	{
		int errorCode = 0;
		errorCode = errno;
		close(ListenSocket);
		return errorCode;
	}
	return SUCCESS_STATUS;
}