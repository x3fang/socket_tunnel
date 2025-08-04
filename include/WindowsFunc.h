#pragma once
#include "../include/globalDefine.h"
WSADATA g_wsaData;
int initServer(SOCKET& ListenSocket, WSADATA& wsaData, sockaddr_in& sockAddr, const int& port, const std::string& connectIp = "0.0.0.0")
{
	int iResult = 0;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		return iResult;
	}
	ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ListenSocket == INVALID_SOCKET)
	{
		int errorCode = 0;
		errorCode = WSAGetLastError();
		WSACleanup();
		return errorCode;
	}
	sockAddr.sin_family = AF_INET;
	inet_pton(AF_INET, connectIp.c_str(), &sockAddr.sin_addr);
	sockAddr.sin_port = htons(port);
	iResult = bind(ListenSocket, (sockaddr*)&sockAddr, sizeof(sockAddr));
	if (iResult == SOCKET_ERROR)
	{
		int errorCode = 0;
		errorCode = WSAGetLastError();
		WSACleanup();
		closesocket(ListenSocket);
		return errorCode;
	}
	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		int errorCode = 0;
		errorCode = WSAGetLastError();
		WSACleanup();
		closesocket(ListenSocket);
		return errorCode;
	}
	return SUCCESS_STATUS;
}