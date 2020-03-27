#pragma once
#include <llvm/ADT/DenseMap.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Value.h>

//#include "llvm2cpg/Logger/CPGLogger.h"

namespace llvm {
class Module;
class Function;
}

namespace llvm2cpg {
class CPGLogger;
class Transforms;

class Transforms {
public:
  Transforms(CPGLogger &logger, bool inlineAP, bool simplify, bool inlineStrings);
  void transformBitcode(llvm::Module &bitcode);

private:
  CPGLogger &logger;
  bool inlineAP; // whether to inline access paths
  bool simplify; // whether to simplify code
  bool inlineStrings; // whether to inline global strings
  void runPasses(llvm::Module &bitcode);
  void markObjCTypeHints(llvm::Module &bitcode);
  void renameOpaqueObjCTypes(llvm::Module &bitcode);
  void inlineGlobalStrings(llvm::Module &bitcode);
};

} // namespace llvm2cpg