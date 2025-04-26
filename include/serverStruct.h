#include "globalDefine.h"
#include "log.h"
#include "Plugin.h"
struct PluginInfoStruct
{
      PluginNamespace::PluginManager *pluginManager;
      Log *log;
      PluginInfoStruct(const PluginInfoStruct &other)
      {
            this->pluginManager = other.pluginManager;
            this->log = other.log;
      }
      ~PluginInfoStruct() = default;
      PluginInfoStruct() : pluginManager(&::pluginManager),
                           log(&g_log) {}
};