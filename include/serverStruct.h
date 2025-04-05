#include "globalDefine.h"
#include "log.h"
#include "Plugin.h"
struct PluginInfoStruct
{
      std::shared_ptr<PluginNamespace::PluginManager> pluginManager;
      std::shared_ptr<Log> log;
      PluginInfoStruct(const PluginInfoStruct &other)
      {
            this->pluginManager = other.pluginManager;
            this->log = other.log;
      }
      ~PluginInfoStruct() = default;
      PluginInfoStruct() : pluginManager(std::make_shared<PluginNamespace::PluginManager>(::pluginManager)),
                           log(std::make_shared<Log>(g_log)) {}
};