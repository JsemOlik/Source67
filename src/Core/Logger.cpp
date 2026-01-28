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


namespace S67 {

std::shared_ptr<spdlog::logger> Logger::s_CoreLogger;
std::shared_ptr<spdlog::logger> Logger::s_ClientLogger;
std::vector<LogEntry> Logger::s_LogHistory;

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

void Logger::Init() {
  spdlog::set_pattern("%^[%T] %n: %v%$");

  // --- Log Rotation ---
  std::filesystem::path logDir = "logs";
  if (!std::filesystem::exists(logDir)) {
    std::filesystem::create_directories(logDir);
  }

  std::vector<std::filesystem::path> logFiles;
  for (const auto &entry : std::filesystem::directory_iterator(logDir)) {
    if (entry.path().extension() == ".txt" &&
        entry.path().filename().string().find("Source67_") == 0) {
      logFiles.push_back(entry.path());
    }
  }

  // Keep only the most recent 9 logs (we're about to create the 10th)
  if (logFiles.size() >= 10) {
    std::sort(logFiles.begin(), logFiles.end());
    size_t toDelete = logFiles.size() - 9;
    for (size_t i = 0; i < toDelete; ++i) {
      std::filesystem::remove(logFiles[i]);
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
  ss << "logs/Source67_" << std::put_time(&timeinfo, "%Y-%m-%d_%H-%M-%S")
     << ".txt";
  std::string logFilename = ss.str();

  // --- Sinks ---
  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  auto imgui_sink = std::make_shared<ImGuiSink_mt>();
  auto file_sink =
      std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilename, true);

  s_CoreLogger = std::make_shared<spdlog::logger>(
      "CORE", spdlog::sinks_init_list{console_sink, imgui_sink, file_sink});
  s_CoreLogger->set_level(spdlog::level::trace);

  s_ClientLogger = std::make_shared<spdlog::logger>(
      "APP", spdlog::sinks_init_list{console_sink, imgui_sink, file_sink});
  s_ClientLogger->set_level(spdlog::level::trace);

  S67_CORE_INFO("Logger initialized. Log file: {0}", logFilename);
}

} // namespace S67
