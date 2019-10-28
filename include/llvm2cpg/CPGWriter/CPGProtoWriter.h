#pragma once

#include <llvm2cpg/CPGWriter/CPGWriter.h>
#include <string>

namespace llvm2cpg {

class CPGLogger;

class CPGProtoWriter : public CPGWriter {
public:
  explicit CPGProtoWriter(CPGLogger &logger, std::string outputDir, std::string outputName);
  void writeCpg(const CPG &cpg) override;

private:
  CPGLogger &logger;
  std::string outputDir;
  std::string outputName;
};

} // namespace llvm2cpg
