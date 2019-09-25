#pragma once

#include "CPGProtoBuilder.h"

namespace llvm2cpg {

class CPG;

class CPGProtoAdapter {
public:
  CPGProtoAdapter(std::string zipPath, bool debug);
  void writeCpg(const llvm2cpg::CPG &cpg);

private:
  bool debug;
  std::string zipPath;
  CPGProtoBuilder builder;

  void saveToArchive();
};

} // namespace llvm2cpg