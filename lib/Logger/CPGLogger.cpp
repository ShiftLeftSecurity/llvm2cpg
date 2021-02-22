#include "llvm2cpg/Logger/CPGLogger.h"
#include <llvm/Support/FileSystem.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

using namespace llvm2cpg;

static std::string getDebugLogPath() {
  llvm::Twine model("llvm2cpg-%%%%%%.log");
  llvm::SmallString<128> output;
  llvm::sys::fs::createUniquePath(model, output, true);
  return output.str().str();
}

static std::shared_ptr<spdlog::sinks::sink> stdoutSink() {
  return std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
}

static std::shared_ptr<spdlog::sinks::sink> fileSink(const std::string &path) {
  return std::make_shared<spdlog::sinks::basic_file_sink_mt>(path);
}

CPGLogger::CPGLogger(bool strictMode)
    : strictModeOn(strictMode), debugLogPath(getDebugLogPath()),
      stdoutLog(std::make_shared<spdlog::logger>("llvm2cpg", stdoutSink())),
      fileLog(std::make_shared<spdlog::logger>("file", fileSink(debugLogPath))) {
  fileLog->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
  fileLog->flush_on(spdlog::level::info);
  fileLog->flush_on(spdlog::level::warn);
}

void CPGLogger::uiInfo(const std::string &message) {
  stdoutLog->info(message);
}

void CPGLogger::uiWarning(const std::string &message) {
  if (strictModeOn) {
    uiFatal(message);
  }
  stdoutLog->warn(message);
}

void CPGLogger::uiFatal(const std::string &message) {
  stdoutLog->critical(message);
  if (strictModeOn) {
    exit(1);
  }
}

void CPGLogger::logInfo(const std::string &message) {
  fileLog->info(message);
}

void CPGLogger::logWarning(const std::string &message) {
  fileLog->warn(message);
}

const std::string &CPGLogger::getLogPath() {
  return debugLogPath;
}

void CPGLogger::doNothing() {
  return;
}