#pragma once

#include <spdlog/logger.h>
#include <string>

namespace llvm2cpg {

class CPGLogger {
public:
  CPGLogger();

  void info(const std::string &message);
  void warning(const std::string &message);
  void error(const std::string &message);
  void fatal(const std::string &message);
  void debug(const std::string &message);

private:
  std::shared_ptr<spdlog::logger> stdoutLog;
  std::shared_ptr<spdlog::logger> debugLog;
};

} // namespace llvm2cpg
