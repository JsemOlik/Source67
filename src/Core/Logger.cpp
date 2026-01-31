#include "Logger.h"
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <iomanip>
#include <mutex>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <sstream>

#ifdef _WIN32
#include <shlobj.h>  // For SHGetKnownFolderPath
#include <windows.h>
#endif


namespace S67 {

std::shared_ptr<spdlog::logger> Logger::s_CoreLogger;
std::shared_ptr<spdlog::logger> Logger::s_ClientLogger;
std::vector<LogEntry> Logger::s_LogHistory;
std::mutex Logger::s_LogMutex;

template <typename Mutex>
class ImGuiSink : public spdlog::sinks::base_sink<Mutex> {
protected:
  void sink_it_(const spdlog::details::log_msg &msg) override {
    spdlog::memory_buf_t formatted;
    spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);

    std::string message = fmt::to_string(formatted);

    // Extract timestamp
    auto time = std::chrono::system_clock::to_time_t(msg.time);
    std::stringstream ss;
    struct tm timeinfo;
#ifdef _WIN32
    localtime_s(&timeinfo, &time);
#else
    localtime_r(&time, &timeinfo);
#endif
    ss << std::put_time(&timeinfo, "%H:%M:%S");

    Logger::AddLogEntry({msg.level, message, ss.str()});
  }

  void flush_() override {}
};

using ImGuiSink_mt = ImGuiSink<std::mutex>;

// Helper function to get a user-writable log directory
static std::filesystem::path GetLogDirectory() {
  std::filesystem::path logDir;
  
#ifdef _WIN32
  // On Windows, use AppData\Local\Source67\logs
  wchar_t* localAppDataPath = nullptr;
  if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &localAppDataPath))) {
    logDir = std::filesystem::path(localAppDataPath) / "Source67" / "logs";
    CoTaskMemFree(localAppDataPath);
  } else {
    // Fallback to current directory if AppData not available
    logDir = "logs";
  }
#else
  // On Unix-like systems, use ~/.local/share/Source67/logs or current directory
  const char* home = getenv("HOME");
  if (home) {
    logDir = std::filesystem::path(home) / ".local" / "share" / "Source67" / "logs";
  } else {
    logDir = "logs";
  }
#endif

  return logDir;
}

void Logger::Init() {
  spdlog::set_pattern("%^[%T] %n: %v%$");

  // --- Log Rotation ---
  std::filesystem::path logDir = GetLogDirectory();
  
  // Try to create log directory with error handling
  try {
    if (!std::filesystem::exists(logDir)) {
      std::filesystem::create_directories(logDir);
    }
  } catch (const std::filesystem::filesystem_error& e) {
    // If we can't create in the preferred location, fallback to temp
    std::cerr << "Failed to create log directory at " << logDir 
              << ": " << e.what() << std::endl;
    logDir = std::filesystem::temp_directory_path() / "Source67" / "logs";
    try {
      if (!std::filesystem::exists(logDir)) {
        std::filesystem::create_directories(logDir);
      }
    } catch (const std::filesystem::filesystem_error& e2) {
      std::cerr << "Failed to create log directory in temp: " << e2.what() << std::endl;
      // Last resort: current directory
      logDir = "logs";
      try {
        if (!std::filesystem::exists(logDir)) {
          std::filesystem::create_directories(logDir);
        }
      } catch (...) {
        // If all else fails, we'll try to continue without file logging
        std::cerr << "WARNING: Could not create log directory anywhere. File logging disabled." << std::endl;
      }
    }
  }

  std::vector<std::filesystem::path> logFiles;
  try {
    if (std::filesystem::exists(logDir) && std::filesystem::is_directory(logDir)) {
      for (const auto &entry : std::filesystem::directory_iterator(logDir)) {
        if (entry.path().extension() == ".txt" &&
            entry.path().filename().string().find("Source67_") == 0) {
          logFiles.push_back(entry.path());
        }
      }
    }
  } catch (const std::filesystem::filesystem_error& e) {
    std::cerr << "Error scanning log directory: " << e.what() << std::endl;
  }

  // Keep only the most recent 9 logs (we're about to create the 10th)
  if (logFiles.size() >= 10) {
    std::sort(logFiles.begin(), logFiles.end());
    size_t toDelete = logFiles.size() - 9;
    for (size_t i = 0; i < toDelete; ++i) {
      try {
        std::filesystem::remove(logFiles[i]);
      } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Failed to remove old log: " << e.what() << std::endl;
      }
    }
  }

  // New log filename with timestamp
  auto now = std::chrono::system_clock::now();
  auto in_time_t = std::chrono::system_clock::to_time_t(now);
  std::stringstream ss;
  struct tm timeinfo;
#ifdef _WIN32
  localtime_s(&timeinfo, &in_time_t);
#else
  localtime_r(&in_time_t, &timeinfo);
#endif
  ss << logDir.string() << "/Source67_" << std::put_time(&timeinfo, "%Y-%m-%d_%H-%M-%S")
     << ".txt";
  std::string logFilename = ss.str();

  // --- Sinks ---
  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  auto imgui_sink = std::make_shared<ImGuiSink_mt>();
  
  // Try to create file sink, but continue if it fails
  std::shared_ptr<spdlog::sinks::basic_file_sink_mt> file_sink;
  try {
    file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilename, true);
  } catch (const std::exception& e) {
    std::cerr << "Failed to create log file at " << logFilename 
              << ": " << e.what() << std::endl;
    std::cerr << "Continuing without file logging..." << std::endl;
  }

  // Create loggers with or without file sink
  if (file_sink) {
    s_CoreLogger = std::make_shared<spdlog::logger>(
        "CORE", spdlog::sinks_init_list{console_sink, imgui_sink, file_sink});
    s_ClientLogger = std::make_shared<spdlog::logger>(
        "APP", spdlog::sinks_init_list{console_sink, imgui_sink, file_sink});
    s_CoreLogger->set_level(spdlog::level::trace);
    s_ClientLogger->set_level(spdlog::level::trace);
    S67_CORE_INFO("Logger initialized. Log file: {0}", logFilename);
  } else {
    s_CoreLogger = std::make_shared<spdlog::logger>(
        "CORE", spdlog::sinks_init_list{console_sink, imgui_sink});
    s_ClientLogger = std::make_shared<spdlog::logger>(
        "APP", spdlog::sinks_init_list{console_sink, imgui_sink});
    s_CoreLogger->set_level(spdlog::level::trace);
    s_ClientLogger->set_level(spdlog::level::trace);
    S67_CORE_WARN("Logger initialized without file logging (console and ImGui only)");
  }
}

} // namespace S67
