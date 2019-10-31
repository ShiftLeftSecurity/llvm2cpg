#include "llvm2cpg/Logger/CPGLogger.h"
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

using namespace llvm2cpg;

static std::shared_ptr<spdlog::sinks::sink> stdoutSink() {
  return std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
}

static std::shared_ptr<spdlog::sinks::sink> fileSink() {
  return std::make_shared<spdlog::sinks::basic_file_sink_mt>("llvm2cpg.debug.log");
}

CPGLogger::CPGLogger()
    : stdoutLog(std::make_shared<spdlog::logger>("llvm2cpg", stdoutSink())),
      debugLog(std::make_shared<spdlog::logger>("file", fileSink())) {
  debugLog->set_level(spdlog::level::debug);
  debugLog->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
}

void CPGLogger::info(const std::string &message) {
  stdoutLog->info(message);
}

void CPGLogger::warning(const std::string &message) {
  stdoutLog->warn(message);
}

void CPGLogger::error(const std::string &message) {
  stdoutLog->error(message);
}

void CPGLogger::fatal(const std::string &message) {
  stdoutLog->critical(message);
}

void CPGLogger::debug(const std::string &message) {
  debugLog->debug(message);
}
