#include "Logger.h"
#include <chrono>
#include <iomanip>
#include <mutex>
#include <spdlog/sinks/base_sink.h>
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

  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  auto imgui_sink = std::make_shared<ImGuiSink_mt>();

  s_CoreLogger = std::make_shared<spdlog::logger>(
      "CORE", spdlog::sinks_init_list{console_sink, imgui_sink});
  s_CoreLogger->set_level(spdlog::level::trace);

  s_ClientLogger = std::make_shared<spdlog::logger>(
      "APP", spdlog::sinks_init_list{console_sink, imgui_sink});
  s_ClientLogger->set_level(spdlog::level::trace);
}

} // namespace S67
