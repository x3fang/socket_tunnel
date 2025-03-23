/*
@author: x3fang
*/
#ifndef clientPlugin_h
#define clientPlugin_h
#include <map>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "clientDefine.h"
typedef bool (*runFunPtr)(clientPluginInfo);
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

      virtual bool runFun(clientPluginInfo &info) = 0;

protected:
      std::string author, version, pluginName;
};
class PluginManager final : public pluginBase
{
public:
      /*
      this function is used to register plugin
      */
      static const std::pair<bool, const std::string> registerFun(pluginBase *plugin)
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
      static bool runFun(const std::string &funName, clientPluginInfo &info)
      {
            if (findFun(funName))
            {
                  return pluginFunList[funName]->runFun(info);
            }
            return false;
      }
      inline static bool findFun(const std::string &funName)
      {
            return (pluginFunList.find(funName) != pluginFunList.end());
      }
      PluginManager();
      ~PluginManager() = default;
      PluginManager(const PluginManager &other) = delete;
      PluginManager &operator=(const PluginManager &other) = delete;

private:
      static std::map<std::string, pluginBase *> pluginFunList;

private:
      bool runFun(clientPluginInfo &info) override { return true; }
};

PluginManager::PluginManager()
{
      this->author = "x3fang";
      this->version = "1.0.0";
      this->pluginName = "PluginManager";
}
std::map<std::string, pluginBase *> PluginManager::pluginFunList;
#endif