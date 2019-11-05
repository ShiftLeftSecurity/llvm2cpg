#pragma once

#include <spdlog/logger.h>
#include <string>

namespace llvm2cpg {

class CPGLogger {
public:
  CPGLogger();

  void uiInfo(const std::string &message);
  void uiWarning(const std::string &message);
  void uiFatal(const std::string &message);

  void logInfo(const std::string &message);
  void logWarning(const std::string &message);

  const std::string &getLogPath();

private:
  std::string debugLogPath;
  std::shared_ptr<spdlog::logger> stdoutLog;
  std::shared_ptr<spdlog::logger> fileLog;
};

} // namespace llvm2cpg
