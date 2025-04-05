#include "..\include\Plugin.h"
#include <iostream>
class test : public PluginNamespace::pluginBase
{
public:
      bool runFun(PluginNamespace::PluginInfo &info) override
      {
            for (int i = 1; i <= 10; i++)
                  std::cout << "TEST";
            return true;
      }
      test()
      {
            this->used = false;
            this->pluginName = "test";
            this->version = "1.0.0";
            this->author = "x3fang";
      }
};
static test Plugin_Test;
extern "C"
{
      EXPORT bool registerFun(PluginNamespace::registerFunValue registerFun)
      {
            return registerFun(&Plugin_Test).first;
      }
}