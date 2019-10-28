#include "llvm2cpg/Logger/CPGLogger.h"
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

using namespace llvm2cpg;

CPGLogger::CPGLogger()
    : stdoutLog(spdlog::stdout_color_mt("llvm2cpg")),
      debugLog(spdlog::basic_logger_mt("file", "llvm2cpg.debug.log")) {
  debugLog->set_level(spdlog::level::debug);
  debugLog->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
}

void CPGLogger::info(const std::string &message) {
  stdoutLog->info(message);
}

void CPGLogger::warning(const std::string &message) {
  debugLog->warn(message);
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
