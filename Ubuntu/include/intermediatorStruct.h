#include "globalDefine.h"
struct IndividualInfoStruct
{
	std::mutex valueLock;
	std::condition_variable valueChange;
	std::unique_lock<std::mutex> lock;
	bool use = false;
	bool del = false;
	std::string wanIp;
	std::string lanIp;
	std::string SEID;
	int systemKind;
	std::string commit;
	std::shared_ptr<SOCKET> commSocket = nullptr, healthSocket = nullptr;
	int waitLock()
	{
		if (del)
			return SUCCESS_STATUS;
		lock = std::unique_lock<std::mutex>(valueLock);
		bool& inUse = use;
		valueChange.wait(lock, [&inUse]
			{ return (!inUse ? true : false); });

		use = true;
		return SUCCESS_STATUS;
	}
	void unLock()
	{
		use = false;
		lock.unlock();
		valueChange.notify_one();
	}
	IndividualInfoStruct() = default;
	~IndividualInfoStruct() = default;
	IndividualInfoStruct(const std::string SEID,
		const std::string wanIp,
		const std::string lanIp,
		int systemKind,
		const std::string commit,
		SOCKET commSocket,
		SOCKET healthSocket)
	{
		this->SEID = SEID;
		this->wanIp = wanIp;
		this->lanIp = lanIp;
		this->systemKind = systemKind;
		this->commit = commit;
		this->commSocket = std::make_shared<SOCKET>(commSocket);
		this->healthSocket = std::make_shared<SOCKET>(healthSocket);
	}
};
struct healthyBeatInfoStruct
{
	inline bool operator==(const healthyBeatInfoStruct& other)
	{
		return this->SEID == other.SEID && this->server == other.server;
	}
	inline bool operator==(const std::string& other)
	{
		return this->SEID == other;
	}
	healthyBeatInfoStruct& operator=(const healthyBeatInfoStruct& other)
	{
		this->healthSocket = other.healthSocket;
		this->SEID = other.SEID;
		this->server = other.server;
		return *this;
	}
	std::string SEID;
	SOCKET& healthSocket;
	bool server = false;
	healthyBeatInfoStruct() = default;
	~healthyBeatInfoStruct() = default;
	healthyBeatInfoStruct(const std::string& SEID_, SOCKET& healthSocket_, bool server_ = false) : SEID(SEID_), healthSocket(healthSocket_), server(server_) {}
};
typedef int (*DelClient)(const std::string&);
typedef bool (*Find)(const std::string&, std::shared_ptr<std::map<std::string, std::shared_ptr<IndividualInfoStruct>>>);
struct PluginInfoStruct
{
	DelClient delClient;
	Find find;
	std::shared_ptr<std::map<std::string, std::shared_ptr<IndividualInfoStruct>>> ClientInfo;
	std::shared_ptr<std::map<std::string, std::shared_ptr<IndividualInfoStruct>>> ServerInfo;
	std::shared_ptr<PluginNamespace::PluginManager> pluginManager;
	std::shared_ptr<Log> log;
	PluginInfoStruct(const PluginInfoStruct& other)
	{
		this->ClientInfo = other.ClientInfo;
		this->ServerInfo = other.ServerInfo;
		this->delClient = other.delClient;
		this->find = other.find;
		this->log = other.log;
		this->pluginManager = other.pluginManager;
	}
	PluginInfoStruct(PluginInfoStruct* other)
	{
		this->ClientInfo = other->ClientInfo;
		this->ServerInfo = other->ServerInfo;
		this->delClient = other->delClient;
		this->find = other->find;
		this->log = other->log;
		this->pluginManager = other->pluginManager;
	}
	PluginInfoStruct(std::shared_ptr<void>& other)
	{
		PluginInfoStruct* temp = (PluginInfoStruct*)(other.get());
		this->ClientInfo = temp->ClientInfo;
		this->ServerInfo = temp->ServerInfo;
		this->delClient = temp->delClient;
		this->find = temp->find;
		this->log = temp->log;
		this->pluginManager = temp->pluginManager;
	}
	PluginInfoStruct operator=(const PluginInfoStruct& other)
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
	PluginInfoStruct() : pluginManager(::pluginManager), log(g_log), delClient(nullptr), find(nullptr) {}
};