#pragma once

#include "CPGProtoBuilder.h"

namespace llvm2cpg {

class CPG;
class CPGLogger;

class CPGProtoAdapter {
public:
  CPGProtoAdapter(CPGLogger &logger, std::string zipPath);
  void writeCpg(const llvm2cpg::CPG &cpg);

private:
  CPGLogger &logger;
  std::string zipPath;
  CPGProtoBuilder builder;

  void saveToArchive();
};

} // namespace llvm2cpg