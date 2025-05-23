#ifndef LOG_H
#define LOG_H
#include <map>
#include <fstream>
#include <string>
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <direct.h>
#include <io.h>
#endif
#if __linux__
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#endif
#include <vector>
#include <ctime>
#include <atomic>
#include <mutex>
#include <sys/stat.h>
namespace logNameSpace
{
      struct ENDL;
      // 通过stat结构体 获得文件大小，单位字节
      size_t getFileSize1(const char *fileName)
      {

            if (fileName == NULL)
            {
                  return 0;
            }

            struct stat statbuf;

            ::stat(fileName, &statbuf);

            size_t filesize = statbuf.st_size;

            return filesize;
      }

      class Log;
      class funLog
      {
      public:
            funLog() = default;
            funLog(const std::string &name, Log *log);
            void write(const std::string &msg);
            void writeln(const std::string &msg);
            void writeln(const int &msg);
            void write(std::vector<std::string> &msg);
            funLog &operator<<(const std::string &msg);
            funLog &operator<<(const int &msg);
            funLog &operator<<(const char &msg);
            funLog &operator<<(ENDL &e)
            {
                  write("\n");
                  return *this;
            }

      private:
            std::shared_ptr<Log> funlog;
            std::string name;
            Log *log;
      };
      class Log
      {
      public:
            Log();
            Log(const Log &other)
            {
                  this->msg = other.msg;
                  this->logName = other.logName;
                  this->logMaxSize = other.logMaxSize;
                  this->writeFlie = std::move(other.writeFlie);
                  this->funlogList = other.funlogList;
            }
            Log(const std::string name, int logMaxSize = 2048);
            ~Log();
            Log *operator=(Log &other);
            Log *operator+(ENDL endl);
            Log &operator=(Log &&other) noexcept;
            void mustChangeFlie();
            void write(const std::string &msg);
            void writeln(const std::string &msg);
            void write(std::vector<std::string> &msg);
            void setName(const std::string &name) { this->logName = name; }
            Log &operator<<(const std::string &msg);
            Log &operator<<(const int msg);
            Log &operator<<(ENDL &e)
            {
                  write("\n");
                  return *this;
            }
            std::shared_ptr<funLog> getFunLog(const std::string &name);

      private:
            std::string msg;
            int logMaxSize = 1024;
            std::shared_ptr<std::atomic<bool>> writeFlie;
            const std::string getTime(void) const;
            bool createDirectory(std::string folder);
            std::ofstream logFile;
            std::string logName;
            std::map<std::string, std::shared_ptr<funLog>> funlogList;
      };

      struct ENDL
      {
      } endl;

      // funLog fun
      funLog::funLog(const std::string &name, Log *log)
      {
            this->funlog = std::shared_ptr<Log>(std::make_shared<Log>(name));
            this->name = name;
            this->log = log;
      }
      void funLog::write(std::vector<std::string> &msg)
      {
            for (auto it = msg.begin(); it != msg.end(); ++it)
            {
                  *it = this->name + " : " + *it;
            }
            funlog->write(msg);
            this->log->write(msg);
            return;
      }
      void funLog::writeln(const std::string &msg)
      {
            funlog->writeln(msg);
            log->writeln(msg);
      }
      void funLog::writeln(const int &msg)
      {
            funlog->writeln(std::to_string(msg));
            log->writeln(std::to_string(msg));
      }
      void funLog::write(const std::string &msg)
      {
            funlog->write(msg);
            log->write(msg);
      }
      funLog &funLog::operator<<(const std::string &msg)
      {
            funlog->write(msg);
            this->log->write(msg);
            return *this;
      }
      funLog &funLog::operator<<(const int &msg)
      {
            funlog->write(std::to_string(msg));
            this->log->write(std::to_string(msg));
            return *this;
      }
      funLog &funLog::operator<<(const char &msg)
      {
            funlog->write(std::string(1, msg));
            this->log->write(std::string(1, msg));
            return *this;
      }

      Log::Log()
      {
            this->writeFlie = std::shared_ptr<std::atomic<bool>>(std::make_shared<std::atomic<bool>>(false));
            this->logName = "NULL";
            this->logMaxSize = 1024 * 1024 * 128;

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
            createDirectory("log");
#endif
#if __linux__
            createDirectory("/" + logName + "-log");
#endif
      }
      // Log fun
      Log::Log(const std::string name, int logMaxSize)
      {
            this->writeFlie = std::shared_ptr<std::atomic<bool>>(std::make_shared<std::atomic<bool>>(false));
            this->logName = name;
            this->logMaxSize = logMaxSize;
            mustChangeFlie();

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
            createDirectory("log");
#endif
#if __linux__
            createDirectory("/" + logName + "-log");
#endif
      }
      Log::~Log()
      {
            if (!msg.empty())
            {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
                  std::ofstream logFilet("log\\" + logName + ".log", std::ios::out | std::ios::app);
#endif
#if __linux__
                  std::ofstream logFilet("/" + logName + "-log/" + logName + ".log", std::ios::out | std::ios::app);
#endif
                  logFilet << getTime() << " " << msg << std::endl;
                  logFilet.close();
            }
      }
      Log *Log::operator=(Log &other)
      {
            return &other;
      }
      Log &Log::operator=(Log &&other) noexcept
      {
            this->msg = other.msg;
            this->logName = other.logName;
            this->logMaxSize = other.logMaxSize;
            this->writeFlie = std::move(other.writeFlie);
            this->funlogList = other.funlogList;
            return *this;
      }
      void Log::mustChangeFlie()
      {
            std::string temp, paths;
            temp = logName;
            int flieSize = 0;
            for (int i = 0; true; i++)
            {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
                  paths = std::string("log\\") + temp + std::string(".log");
#endif
#if __linux__
                  paths = std::string("/log/") + temp + std::string(".log");
#endif
                  flieSize = getFileSize1(paths.c_str());
                  if (flieSize >= logMaxSize)
                  {
                        if (i > 0)
                              temp = temp.substr(0, temp.length() - 3) + "_" + std::to_string(i);
                        else
                              temp = temp + "_0";
                  }
                  else
                        break;
            }
            logName = temp;
      }
      void Log::writeln(const std::string &msg)
      {
            this->write(msg + "\n");
            return;
      }
      void Log::write(const std::string &msg)
      {
            try
            {
                  this->msg += msg;
            }
            catch (...)
            {
                  return;
            }
            if (this->msg.find("\n") != std::string::npos)
            {
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
                  std::ofstream logFilet("log\\" + logName + ".log", std::ios::out | std::ios::app);
#endif
#if __linux__
                  std::ofstream logFilet("/" + logName + "-log/" + logName + ".log", std::ios::out | std::ios::app);
#endif
                  logFilet << getTime() << " " << this->msg;
                  logFilet.close();
                  this->msg = (this->msg.find_last_of("\n") == this->msg.length() - 1 ? "" : this->msg.substr(this->msg.find_last_of("\n") + 1));
            }
            mustChangeFlie();
            return;
      }
      void Log::write(std::vector<std::string> &msg)
      {
            for (auto it = msg.begin(); it != msg.end(); ++it)
            {
                  write(*it);
            }
            mustChangeFlie();
            return;
      }
      Log &Log::operator<<(const std::string &msg)
      {
            while (this->writeFlie->exchange(true, std::memory_order_acquire))
                  ;
            write(std::string(msg));
            this->writeFlie->store(false, std::memory_order_release);
            return *this;
      }
      Log &Log::operator<<(const int msg)
      {
            while (this->writeFlie->exchange(true, std::memory_order_acquire))
                  ;
            write(std::to_string(msg));
            this->writeFlie->store(false, std::memory_order_release);
            return *this;
      }

      std::shared_ptr<funLog> Log::getFunLog(const std::string &name)
      {
            if (funlogList.find(name) == funlogList.end())
                  funlogList[name] = std::make_shared<funLog>(name, this);
            return funlogList[name];
      }
      const std::string Log::getTime(void) const
      {
            time_t now = time(NULL);
            tm *tm_t = localtime(&now);
            return ('[' + std::to_string(tm_t->tm_year + 1900) +
                    "/" + std::to_string(tm_t->tm_mon + 1) +
                    "/" + std::to_string(tm_t->tm_mday) +
                    ":" + std::to_string(tm_t->tm_hour) +
                    ":" + std::to_string(tm_t->tm_min) +
                    ":" + std::to_string(tm_t->tm_sec) + ']');
      }
      bool Log::createDirectory(std::string folder)
      {
            std::string folder_builder;
            std::string sub;
            sub.reserve(folder.size());
            for (auto it = folder.begin(); it != folder.end(); ++it)
            {
                  // cout << *(folder.end()-1) << endl;
                  const char c = *it;
                  sub.push_back(c);
                  if (c == '\\' || it == folder.end() - 1)
                  {
                        folder_builder.append(sub);

                        // this folder not exist
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
                        if (0 != ::_access(folder_builder.c_str(), 0))
                        {
                              if (0 != ::mkdir(folder_builder.c_str()))
#elif __linux__
                        if (0 != ::access(folder_builder.c_str(), 0))
                        {
                              if (0 != ::mkdir(folder_builder.c_str(), 777))
#endif
                              {
                                    // create failed
                                    return false;
                              }
                        }
                        sub.clear();
                  }
            }
            return true;
      }
      std::string operator+(std::string msg, ENDL p)
      {
            return msg + "\n";
      }
};

#endif