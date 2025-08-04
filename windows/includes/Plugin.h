/*
@author: x3fang
*/
#ifndef clientPlugin_h
#define clientPlugin_h
#include <map>
#include <string>
#include <locale>
#include <queue>
#include <sstream>
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define windowsSystem true
#include <windows.h>
#endif
#if !windowsSystem
#include <dirent.h>
#include <dlfcn.h>
#endif

namespace PluginNamespace
{
	std::vector<std::string> traverseFiles(const std::string& rootDir, const std::vector<std::string>& compareSuffix, const std::vector<std::string>& blockWords, std::vector<std::string>* fileNameVector = nullptr);
	struct Info
	{
	private:
		using data_type = std::vector<std::shared_ptr<void>>;
		std::vector<std::string> arguments_splitBySpaceList;
		std::string arguments;

	public:
		data_type customize_data;
		Info() = default;
		Info(const data_type& n_data)
			: customize_data(n_data)
		{
		}
		Info(const Info& other)
			: customize_data(other.customize_data)
		{
		}
		void setArguments(const std::string& n_arguments)
		{
			arguments = n_arguments;
			arguments_splitBySpaceList.clear();
			std::istringstream ss(arguments);
			std::string temp;
			while (ss >> temp)
			{
				arguments_splitBySpaceList.push_back(temp);
			}
			return;
		}
		inline std::vector<std::string> getArguments_splitBySpaceList() const
		{
			return arguments_splitBySpaceList;
		}
		inline std::string getArguments() const
		{
			return arguments;
		}
		inline Info& operator=(const Info& other)
		{
			customize_data = other.customize_data;
			arguments = other.arguments;
			arguments_splitBySpaceList = other.arguments_splitBySpaceList;
			return *this;
		}
	};
	class PluginManager;
	class pluginBase
	{
	public:
		pluginBase() = default;
		~pluginBase() = default;
		pluginBase(const pluginBase& other) = delete;
		pluginBase& operator=(const pluginBase& other) = delete;
		inline const std::string getAuthor(void) const { return this->author; }
		inline const std::string getVersion(void) const { return this->version; }
		inline const std::string getPluginName(void) const { return this->pluginName; }

		virtual bool runFun(Info& info) = 0;
		bool used = false;
		bool show = true;
		bool local = false;

	protected:
		std::string author, version, pluginName;
	};
	class PluginManager final : public pluginBase
	{
	public:
		/*
		this function is used to register plugin
		*/
		const std::pair<bool, const std::string> registerFun(pluginBase* plugin)
		{
			if (!plugin)
				return std::pair<bool, const std::string>(false, "");
			const std::string pluginAuthor = plugin->getAuthor();
			const std::string pluginName = plugin->getPluginName();
			if (!pluginAuthor.empty() && !pluginName.empty() && !findFun(pluginAuthor)) // not found,register
			{
				auto res = pluginFunList.insert(std::pair<std::string, pluginBase*>(pluginName, plugin));
				if (res.second) // inserted success
					return std::pair<bool, const std::string>(true, pluginAuthor);
				else
					return std::pair<bool, const std::string>(false, pluginAuthor);
			}
			else // found same name plugin
				return std::pair<bool, const std::string>(false, pluginAuthor);
		}
		void unregisterFun(const std::string& funName)
		{
			pluginFunList.erase(funName);
		}
		bool runFun(const std::string& funName, Info& info)
		{
			if (findFun(funName) && pluginFunList[funName]->used)
				return pluginFunList[funName]->runFun(info);
			return false;
		}
		std::vector<std::string> getAllPluginName()
		{
			std::vector<std::string> res;
			for (auto& it : pluginFunList)
			{
				if (pluginFunList[it.first]->used && pluginFunList[it.first]->show)
					res.push_back(it.first);
			}
			return res;
		}
		inline bool findFun(const std::string& funName)
		{
			return (pluginFunList.find(funName) != pluginFunList.end());
		}
		inline std::size_t getLoadPluginNum() { return pluginFunList.size(); }
		std::vector<std::string> getLocalPluginName(void)
		{
			std::vector<std::string> res;
			for (auto& it : pluginFunList)
			{
				if (it.second->local && it.second->show)
					res.push_back(it.first);
			}
			return res;
		}
		inline const std::string getFunAuthor(const std::string& funName)
		{
			return (findFun(funName) ? pluginFunList[funName]->getAuthor() : std::string(""));
		}
		inline const std::string getFunVersion(const std::string& funName)
		{
			return (findFun(funName) ? pluginFunList[funName]->getVersion() : std::string(""));
		}
		inline const bool isFunLocal(const std::string& funName)
		{
			return (findFun(funName) ? pluginFunList[funName]->local : false);
		}
		inline const bool isFunUsed(const std::string& funName)
		{
			return (findFun(funName) ? pluginFunList[funName]->used : false);
		}
		inline const bool isFunShow(const std::string& funName)
		{
			return (findFun(funName) ? pluginFunList[funName]->show : false);
		}

		PluginManager();
		~PluginManager() = default;
		PluginManager(PluginManager* other)
		{
			this->used = other->used;
			this->author = other->author;
			this->version = other->version;
			this->pluginName = other->pluginName;
			this->pluginFunList = other->pluginFunList;
		}
		PluginManager(const PluginManager& other)
		{
			this->used = other.used;
			this->author = other.author;
			this->version = other.version;
			this->pluginName = other.pluginName;
			this->pluginFunList = other.pluginFunList;
		}
		PluginManager operator=(const PluginManager& other)
		{
			this->used = other.used;
			this->author = other.author;
			this->version = other.version;
			this->pluginName = other.pluginName;
			this->pluginFunList = other.pluginFunList;
			return *this;
		}

	private:
		std::map<std::string, pluginBase*> pluginFunList;

	private:
		bool runFun(Info& info) override
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

	typedef bool (*registerFun)(PluginManager&);
	typedef bool (*unregisterFun)(PluginManager&);
#if windowsSystem
	static std::vector<HINSTANCE> pluginDllHandle;
#else
	static std::vector<void*> pluginDllHandle;
#endif
	int loadPlugin(PluginManager& manager, const std::string& pluginPath, const std::string& pluginExtension)
	{
		int res = 0;
		std::vector<std::string> pluginExtensionVector;
		std::vector<std::string> blockWords;

		pluginExtensionVector.push_back(std::string(pluginExtension));

		auto pluginPathVector = traverseFiles(pluginPath, pluginExtensionVector, blockWords);
		for (auto it = pluginPathVector.begin(); it != pluginPathVector.end(); it++)
		{
#if windowsSystem
			pluginDllHandle.push_back((HINSTANCE)LoadLibraryA(it->c_str()));
#else
			pluginDllHandle.push_back(dlopen(it->c_str(), RTLD_LAZY));
#endif
			if (!pluginDllHandle.back())
			{
				pluginDllHandle.pop_back();
				continue;
			}
#if windowsSystem
			auto regFun = ((registerFun)GetProcAddress(pluginDllHandle.back(), "registerFun"));
#else
			auto regFun = ((registerFun)dlsym(pluginDllHandle.back(), "registerFun"));
#endif

			if (regFun)
			{
				if (regFun(manager))
					res++;
			}
		}
		return res;
	}
	void unloadPlugin(PluginManager& manager)
	{
		for (auto& it : pluginDllHandle)
		{
#if windowsSystem
			auto regFun = ((unregisterFun)GetProcAddress(pluginDllHandle.back(), "unregisterFun"));
#else
			auto regFun = ((unregisterFun)dlsym(pluginDllHandle.back(), "unregisterFun"));
#endif
			if (regFun)
			{
				regFun(manager);
#if windowsSystem
				CloseHandle(it);
#else
				dlclose(it);
#endif
			}
		}
	}
#if windowsSystem
	std::vector<std::wstring> stringVector_TO_wstringVector(const std::vector<std::string>& strVec)
	{
		std::vector<std::wstring> wstrVec;
		for (const auto& s : strVec)
		{
			wstrVec.push_back(std::wstring(s.begin(), s.end()));
		}
		return wstrVec;
	}
	std::string UTF16ToUTF8(const std::wstring& utf16)
	{
		if (utf16.empty())
			return "";

		int size = WideCharToMultiByte(
			CP_UTF8, 0,
			utf16.c_str(), -1,
			nullptr, 0,
			nullptr, nullptr);

		std::string utf8(size, 0);
		WideCharToMultiByte(
			CP_UTF8, 0,
			utf16.c_str(), -1,
			&utf8[0], size,
			nullptr, nullptr);

		return utf8;
	}
	std::vector<std::string> traverseFiles(const std::string& rootDir, const std::vector<std::string>& compareSuffix, const std::vector<std::string>& blockWords, std::vector<std::string>* fileNameVector)
	{
		std::vector<std::string> filePaths;
		std::queue<std::wstring> dirQueue;
		dirQueue.push(std::wstring(rootDir.begin(), rootDir.end()));

		WIN32_FIND_DATAW findData;
		HANDLE hFind;

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

				std::string fileExt = UTF16ToUTF8(std::wstring(findData.cFileName));
				fileExt = fileExt.substr(fileExt.find_last_of(".") + 1);
				fileExt.pop_back();//pop '\0'

				if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					dirQueue.push(fullPath);
				}
				else if (fullPath.find_last_of(L".") != std::wstring::npos &&
					std::find_if(blockWords.begin(), blockWords.end(),
						[&](const std::string& word)
						{ return UTF16ToUTF8(fullPath).find(word) != std::string::npos; })
					== blockWords.end() &&
					std::find(compareSuffix.begin(),
						compareSuffix.end(), fileExt) != compareSuffix.end())
				{
					// 转换为UTF-8并存入vector
					try
					{
						filePaths.emplace_back(UTF16ToUTF8(fullPath));
						if (fileNameVector)
						{
							std::wstring fileName = std::wstring(findData.cFileName);
							fileNameVector->push_back(UTF16ToUTF8(fileName));
						}
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
#else
	std::vector<std::string> traverseFiles(const std::string& rootDir,
		const std::vector<std::string>& compareSuffix,
		const std::vector<std::string>& blockWords,
		std::vector<std::string>* fileNameVector)
	{
		std::vector<std::string> filePaths;
		std::queue<std::string> dirQueue;
		dirQueue.push(rootDir);

		while (!dirQueue.empty())
		{
			std::string currentDir = std::move(dirQueue.front());
			dirQueue.pop();

			DIR* dir = opendir(currentDir.c_str());
			if (!dir)
				continue;

			struct dirent* entry;
			while ((entry = readdir(dir)) != nullptr)
			{
				// 跳过 "." 和 ".."
				if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
				{
					continue;
				}

				// 构建完整路径
				std::string fullPath = currentDir;
				if (fullPath.back() != '/')
					fullPath += '/';
				fullPath += entry->d_name;

				// 获取文件类型
				bool isDir = false;
				bool isReg = false;
				struct stat statBuf;

				switch (entry->d_type)
				{
				case DT_DIR:
					isDir = true;
					break;
				case DT_REG:
					isReg = true;
					break;
				case DT_LNK: // 跳过符号链接
					continue;
				case DT_UNKNOWN: // 需要手动检查
					if (lstat(fullPath.c_str(), &statBuf) == 0)
					{
						if (S_ISDIR(statBuf.st_mode))
							isDir = true;
						else if (S_ISREG(statBuf.st_mode))
							isReg = true;
					}
					break;
				default: // 跳过其他类型
					continue;
				}

				if (isDir)
				{
					dirQueue.push(fullPath);
				}
				else if (isReg)
				{
					// 检查扩展名
					size_t dotPos = fullPath.find_last_of('.');
					if (dotPos == std::string::npos)
						continue;

					std::string ext = fullPath.substr(dotPos + 1);
					if (std::find(compareSuffix.begin(), compareSuffix.end(), ext) == compareSuffix.end())
					{
						continue;
					}

					// 检查屏蔽词
					bool blocked = false;
					for (const auto& word : blockWords)
					{
						if (fullPath.find(word) != std::string::npos)
						{
							blocked = true;
							break;
						}
					}
					if (blocked)
						continue;

					// 添加到结果
					filePaths.push_back(fullPath);
					if (fileNameVector)
					{
						fileNameVector->push_back(entry->d_name);
					}
				}
			}
			closedir(dir);
		}

		return filePaths;
	}
#endif
}
#endif