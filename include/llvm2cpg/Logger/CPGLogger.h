#pragma once

#include <spdlog/logger.h>
#include <string>

namespace llvm2cpg {

class CPGLogger {
public:
  explicit CPGLogger(bool strictMode);

  void uiInfo(const std::string &message);
  void uiWarning(const std::string &message);
  void uiFatal(const std::string &message);

  void logInfo(const std::string &message);
  void logWarning(const std::string &message);

  void doNothing(); // serves to get rid of "unused variable" warnings

  const std::string &getLogPath();

private:
  bool strictModeOn;
  std::string debugLogPath;
  std::shared_ptr<spdlog::logger> stdoutLog;
  std::shared_ptr<spdlog::logger> fileLog;
};

} // namespace llvm2cpg
