/*
@author: x3fang
*/
#ifndef clientPlugin_h
#define clientPlugin_h
#include <map>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <locale>
#include <codecvt>
#include <queue>
#include <iostream>
#include "globalDefine.h"

namespace PluginNamespace
{
      std::vector<std::string> traverseFiles(const std::wstring &rootDir, const std::wstring &compareSuffix);
      struct Info
      {
      private:
            using data_type = std::vector<std::shared_ptr<void>>;

      public:
            data_type customize_data;
            Info() = default;
            Info(const data_type &n_data)
                : customize_data(n_data) {}
            Info(const Info &other)
                : customize_data(other.customize_data) {}
            inline Info &operator=(const Info &other)
            {
                  customize_data = other.customize_data;
                  return *this;
            }
      };
      class pluginBase
      {
      public:
            pluginBase() = default;
            ~pluginBase() = default;
            pluginBase(const pluginBase &other) = delete;
            pluginBase &operator=(const pluginBase &other) = delete;
            inline const std::string getAuthor(void) const { return this->author; }
            inline const std::string getversion(void) const { return this->version; }
            inline const std::string getpluginName(void) const { return this->pluginName; }

            virtual bool runFun(Info &info) = 0;
            bool used = false;
            bool show = true;

      protected:
            std::string author, version, pluginName;
      };
      class PluginManager final : public pluginBase
      {
      public:
            /*
            this function is used to register plugin
            */
            const std::pair<bool, const std::string> registerFun(pluginBase *plugin)
            {
                  if (!plugin)
                        return std::pair<bool, const std::string>(false, "");
                  const std::string pluginAuthor = plugin->getAuthor();
                  const std::string pluginName = plugin->getpluginName();
                  if (!pluginAuthor.empty() && !pluginName.empty() && !findFun(pluginAuthor)) // not found,register
                  {
                        auto res = pluginFunList.insert(std::pair<std::string, pluginBase *>(pluginName, plugin));
                        if (res.second) // inserted success
                              return std::pair<bool, const std::string>(true, pluginAuthor);
                        else
                              return std::pair<bool, const std::string>(false, pluginAuthor);
                  }
                  else // found same name plugin
                        return std::pair<bool, const std::string>(false, pluginAuthor);
            }
            bool runFun(const std::string &funName, Info &info)
            {
                  if (findFun(funName) && pluginFunList[funName]->used)
                        return pluginFunList[funName]->runFun(info);
                  return false;
            }
            std::vector<std::string> getAllPluginName()
            {
                  std::vector<std::string> res;
                  for (auto &it : pluginFunList)
                  {
                        if (pluginFunList[it.first]->used && pluginFunList[it.first]->show)
                              res.push_back(it.first);
                  }
                  return res;
            }
            inline bool findFun(const std::string &funName)
            {
                  return (pluginFunList.find(funName) != pluginFunList.end());
            }
            inline std::size_t getLoadPluginNum() { return pluginFunList.size(); }
            PluginManager();
            ~PluginManager() = default;
            PluginManager(PluginManager *other)
            {
                  this->used = other->used;
                  this->author = other->author;
                  this->version = other->version;
                  this->pluginName = other->pluginName;
                  this->pluginFunList = other->pluginFunList;
            }
            PluginManager(const PluginManager &other)
            {
                  this->used = other.used;
                  this->author = other.author;
                  this->version = other.version;
                  this->pluginName = other.pluginName;
                  this->pluginFunList = other.pluginFunList;
            }
            PluginManager operator=(const PluginManager &other)
            {
                  this->used = other.used;
                  this->author = other.author;
                  this->version = other.version;
                  this->pluginName = other.pluginName;
                  this->pluginFunList = other.pluginFunList;
                  return *this;
            }

      private:
            std::map<std::string, pluginBase *> pluginFunList;

      private:
            bool runFun(Info &info) override
            {
                  throw "Error at PluginManager::runFun,line:" + __LINE__;
                  return true;
            }
      };
      PluginManager::PluginManager()
      {
            this->used = true;
            this->author = "x3fang";
            this->version = "1.0.0";
            this->pluginName = "PluginManager";
      }
      typedef bool (*registerFun)(PluginManager &);
      std::vector<HINSTANCE> pluginDllHandle(5, 0);
      int loadPlugin(PluginManager &manager, const std::string &pluginPath)
      {
            int res = 0;
            auto pluginPathVector = traverseFiles(std::wstring(pluginPath.begin(), pluginPath.end()), L"dll");
            int i = 0;
            for (auto it = pluginPathVector.begin(); it != pluginPathVector.end(); it++, i++)
            {
                  pluginDllHandle[i] = (HINSTANCE)LoadLibrary(it->c_str());
                  if (pluginDllHandle[i] == NULL)
                  {
                        i--;
                        continue;
                  }

                  auto regFun = ((registerFun)GetProcAddress(pluginDllHandle[i], "registerFun"));
                  if (regFun)
                  {
                        if (regFun(manager))
                              res++;
                  }
            }
            return res;
      }

      std::vector<std::string> traverseFiles(const std::wstring &rootDir, const std::wstring &compareSuffix)
      {
            std::vector<std::string> filePaths;
            std::queue<std::wstring> dirQueue;
            dirQueue.push(rootDir);

            WIN32_FIND_DATAW findData;
            HANDLE hFind;

            // UTF-16 到 UTF-8 转换器
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

            while (!dirQueue.empty())
            {
                  std::wstring currentDir = std::move(dirQueue.front());
                  dirQueue.pop();

                  // 手动构造搜索路径 currentDir\\*
                  std::wstring searchPath = currentDir;
                  if (currentDir.back() != L'\\')
                        searchPath += L'\\';
                  searchPath += L'*';

                  hFind = FindFirstFileW(searchPath.c_str(), &findData);
                  if (hFind == INVALID_HANDLE_VALUE)
                        continue;

                  do
                  {
                        // 跳过 "." 和 ".."
                        if (findData.cFileName[0] == L'.' &&
                            (findData.cFileName[1] == L'\0' ||
                             (findData.cFileName[1] == L'.' && findData.cFileName[2] == L'\0')))
                        {
                              continue;
                        }

                        // 手动拼接完整路径
                        std::wstring fullPath = currentDir;
                        if (fullPath.back() != L'\\')
                              fullPath += L'\\';
                        fullPath += findData.cFileName;

                        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                        {
                              dirQueue.push(fullPath);
                        }
                        else if (fullPath.find_last_of(L".") != std::wstring::npos && fullPath.substr(fullPath.find_last_of(L".") + 1) == compareSuffix)
                        {
                              // 转换为UTF-8并存入vector
                              try
                              {
                                    filePaths.emplace_back(converter.to_bytes(fullPath));
                              }
                              catch (...)
                              {
                                    // 忽略编码转换失败的文件
                              }
                        }
                  } while (FindNextFileW(hFind, &findData));

                  FindClose(hFind);
            }

            return filePaths;
      }
}
#endif