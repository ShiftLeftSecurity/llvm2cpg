#pragma once

#include <llvm2cpg/CPGWriter/CPGWriter.h>
#include <string>

namespace llvm2cpg {

class CPGLogger;

class CPGProtoWriter : public CPGWriter {
public:
  explicit CPGProtoWriter(CPGLogger &logger, std::string output);
  void writeCpg(const CPG &cpg) override;

private:
  CPGLogger &logger;
  std::string output;
};

} // namespace llvm2cpg
