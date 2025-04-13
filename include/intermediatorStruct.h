#include "globalDefine.h"
struct IndividualInfoStruct
{
      bool use = false;
      std::string wanIp;
      std::string lanIp;
      std::string SEID;
      int systemKind;
      std::string commit;
      SOCKET commSocket = INVALID_SOCKET, healthSocket = INVALID_SOCKET;
      IndividualInfoStruct() = default;
      ~IndividualInfoStruct() = default;
      IndividualInfoStruct(const std::string &SEID, const std::string &wanIp, const std::string &lanIp, int systemKind, const std::string &commit, SOCKET commSocket, SOCKET healthSocket)
      {
            this->SEID = SEID;
            this->wanIp = wanIp;
            this->lanIp = lanIp;
            this->systemKind = systemKind;
            this->commit = commit;
            this->commSocket = commSocket;
            this->healthSocket = healthSocket;
      }
};
struct healthyBeatInfoStruct
{
      inline bool operator==(const healthyBeatInfoStruct &other)
      {
            return this->SEID == other.SEID && this->server == other.server;
      }
      inline bool operator==(const std::string &other)
      {
            return this->SEID == other;
      }
      healthyBeatInfoStruct &operator=(const healthyBeatInfoStruct &other)
      {
            this->healthSocket = other.healthSocket;
            this->SEID = other.SEID;
            this->server = other.server;
            return *this;
      }
      std::string SEID;
      SOCKET &healthSocket;
      bool server = false;
      healthyBeatInfoStruct() = default;
      ~healthyBeatInfoStruct() = default;
      healthyBeatInfoStruct(const std::string &SEID_, SOCKET &healthSocket_, bool server_ = false) : SEID(SEID_), healthSocket(healthSocket_), server(server_) {}
};
typedef bool (*DelClient)(const std::string &);
typedef bool (*Find)(const std::string &, std::map<std::string, std::shared_ptr<IndividualInfoStruct>> *);
struct PluginInfoStruct
{
      DelClient delClient;
      Find find;
      std::shared_ptr<std::map<std::string, std::shared_ptr<IndividualInfoStruct>>> ClientInfo;
      std::shared_ptr<std::map<std::string, std::shared_ptr<IndividualInfoStruct>>> ServerInfo;
      std::shared_ptr<PluginNamespace::PluginManager> pluginManager;
      std::shared_ptr<Log> log;
      PluginInfoStruct(const PluginInfoStruct &other)
      {
            this->ClientInfo = other.ClientInfo;
            this->ServerInfo = other.ServerInfo;
            this->delClient = other.delClient;
            this->find = other.find;
            this->log = other.log;
            this->pluginManager = other.pluginManager;
      }
      PluginInfoStruct(PluginInfoStruct *other)
      {
            this->ClientInfo = other->ClientInfo;
            this->ServerInfo = other->ServerInfo;
            this->delClient = other->delClient;
            this->find = other->find;
            this->log = other->log;
            this->pluginManager = other->pluginManager;
      }
      PluginInfoStruct(std::shared_ptr<void> &other)
      {
            PluginInfoStruct *temp = (PluginInfoStruct *)(other.get());
            this->ClientInfo = temp->ClientInfo;
            std::cerr << 1;
            this->ServerInfo = temp->ServerInfo;
            std::cerr << 1;
            this->delClient = temp->delClient;
            std::cerr << 1;
            this->find = temp->find;
            this->log = temp->log;
            this->pluginManager = temp->pluginManager;
      }
      PluginInfoStruct operator=(const PluginInfoStruct &other)
      {
            this->ClientInfo = other.ClientInfo;
            this->ServerInfo = other.ServerInfo;
            this->delClient = other.delClient;
            this->find = other.find;
            this->log = other.log;
            this->pluginManager = other.pluginManager;
            return *this;
      }
      ~PluginInfoStruct() = default;
      PluginInfoStruct() : pluginManager(std::make_shared<PluginNamespace::PluginManager>()), log(std::make_shared<Log>(g_log)) {}
};