#pragma once

#include "CPGProtoBuilder.h"
#include <cstdint>
#include <llvm/IR/Instruction.h>

namespace llvm2cpg {

class CPG;
class CPGMethod;
class ASTNode;

class CPGProtoAdapter {
public:
  CPGProtoAdapter(std::string zipPath, bool debug);
  void writeCpg(const llvm2cpg::CPG &cpg);

private:
  bool debug;
  std::string zipPath;
  CPGProtoBuilder builder;

  void saveToArchive();

  CPGProtoNode *emitValue(const llvm::Value *value);
};

} // namespace llvm2cpg