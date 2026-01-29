#pragma once

#include <memory>

#include <filesystem>
#include <spdlog/fmt/ostr.h>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>

namespace S67 {

struct LogEntry {
  spdlog::level::level_enum Level;
  std::string Message;
  std::string Timestamp;
};

class Logger {
public:
  static void Init();

  inline static std::shared_ptr<spdlog::logger> &GetCoreLogger() {
    return s_CoreLogger;
  }
  inline static std::shared_ptr<spdlog::logger> &GetClientLogger() {
    return s_ClientLogger;
  }

  static const std::vector<LogEntry> GetLogHistory() { 
    std::lock_guard<std::mutex> lock(s_LogMutex);
    return s_LogHistory; 
  }
  
  static void ClearLogHistory() { 
    std::lock_guard<std::mutex> lock(s_LogMutex);
    s_LogHistory.clear(); 
  }
  
  static void AddLogEntry(const LogEntry &entry) {
    std::lock_guard<std::mutex> lock(s_LogMutex);
    s_LogHistory.push_back(entry);
    
    // Limit history size to prevent unbounded growth
    if (s_LogHistory.size() > 10000) {
      s_LogHistory.erase(s_LogHistory.begin());
    }
  }

private:
  static std::shared_ptr<spdlog::logger> s_CoreLogger;
  static std::shared_ptr<spdlog::logger> s_ClientLogger;
  static std::vector<LogEntry> s_LogHistory;
  static std::mutex s_LogMutex;
};

} // namespace S67

// Core log macros
#define S67_CORE_TRACE(...) ::S67::Logger::GetCoreLogger()->trace(__VA_ARGS__)
#define S67_CORE_INFO(...) ::S67::Logger::GetCoreLogger()->info(__VA_ARGS__)
#define S67_CORE_WARN(...) ::S67::Logger::GetCoreLogger()->warn(__VA_ARGS__)
#define S67_CORE_ERROR(...) ::S67::Logger::GetCoreLogger()->error(__VA_ARGS__)
#define S67_CORE_CRITICAL(...)                                                 \
  ::S67::Logger::GetCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define S67_TRACE(...) ::S67::Logger::GetClientLogger()->trace(__VA_ARGS__)
#define S67_INFO(...) ::S67::Logger::GetClientLogger()->info(__VA_ARGS__)
#define S67_WARN(...) ::S67::Logger::GetClientLogger()->warn(__VA_ARGS__)
#define S67_ERROR(...) ::S67::Logger::GetClientLogger()->error(__VA_ARGS__)
#define S67_CRITICAL(...)                                                      \
  ::S67::Logger::GetClientLogger()->critical(__VA_ARGS__)
