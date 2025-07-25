#include "..\include\globalDefine.h"
#include "..\include\serverStruct.h"
class delPage_plugin : public PluginNamespace::pluginBase
{
public:
      bool runFun(PluginNamespace::Info &info) override
      {
            if (info.customize_data.empty())
                  return false;

            std::shared_ptr<programPluginInfoStruct> &funInfo = (std::shared_ptr<programPluginInfoStruct> &)(info.customize_data[0]);
            std::shared_ptr<PluginInfoStruct> &pluginInfo = (std::shared_ptr<PluginInfoStruct> &)(info.customize_data[1]);
            std::shared_ptr<SOCKET> sock = funInfo->mainConnectSocket;
            auto funlog = pluginInfo->log->getFunLog("connectClient");

            funlog->writeln("start delPage");

            while (true)
            {
                  system("cls");
                  funlog->writeln("show client page");
                  auto showClientInfo = info;
                  send(*sock, "showClient");

                  std::string res;
                  recv(*sock, res);
                  if (res == "success")
                  {
                        send(*sock, "use 0 EQUAL |"); // send arguments
                        pluginInfo->pluginManager->runFun("showClient", showClientInfo);

                        recv(*sock, res);
                        if (res != "success")
                        {
                              system("cls");
                              funlog->writeln("server:showClient failed");
                              std::cout << res;
                              std::cout << "fail to run showClient plugin!" << std::endl;
                              std::cout << "The plugin will exit.";
                              system("pause");
                        }

                        int clientNum = *std::static_pointer_cast<int>(showClientInfo.customize_data.back());
                        showClientInfo.customize_data.pop_back();

                        std::shared_ptr<std::vector<std::string>> ClientSEIDList = std::static_pointer_cast<std::vector<std::string>>(showClientInfo.customize_data.back());
                        showClientInfo.customize_data.pop_back();

                        int chooseClientNum = -1;
                        std::cout << "choose client number or input -1 to exit:";
                        std::cin >> chooseClientNum;
                        if (chooseClientNum == -1)
                        {
                              funlog->writeln("exit");
                              break;
                        }
                        else if (chooseClientNum < 0 && chooseClientNum >= clientNum)
                        {
                              std::cout << "choose client number error" << std::endl;
                              system("pause");
                              continue;
                        }
                        else
                        {
                              auto delClientInfo = info;
                              std::string data;
                              delClientInfo.customize_data.push_back(std::make_shared<std::string>(ClientSEIDList->at(chooseClientNum)));
                              send(*sock, "delClient");
                              recv(*sock, data);
                              if (data == "success")
                              {
                                    send(*sock, "NULL"); // send arguments
                                    if (pluginInfo->pluginManager->runFun("delClient", delClientInfo))
                                    {
                                          recv(*sock, data);
                                          if (data == "success")
                                                std::cout << "del client success" << std::endl;
                                          else
                                                std::cout << "del client fail" << std::endl;
                                    }
                                    else
                                    {
                                          std::cout << "del client fail" << std::endl;
                                    }
                                    system("pause");
                              }
                              else
                              {
                                    std::cout << "find server plugin fail" << std::endl;
                                    system("pause");
                              }
                        }
                  }
            }

            return true;
      }
      delPage_plugin()
      {
            this->used = true;
            this->show = true;
            this->pluginName = "delPage_plugin";
            this->version = "1.0.0";
            this->author = "x3fang";
      }
};
static delPage_plugin DelPage_plugin;
extern "C"
{
      EXPORT bool registerFun(PluginNamespace::PluginManager &pluginManager)
      {
            return pluginManager.registerFun(&DelPage_plugin).first;
      }
}