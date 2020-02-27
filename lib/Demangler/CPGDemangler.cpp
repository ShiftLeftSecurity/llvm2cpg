#include "llvm2cpg/Demangler/CPGDemangler.h"
#include "IntrinsicsDemangler.h"
#include <llvm/IR/Function.h>

using namespace llvm2cpg;

static DemangledName demangleIntrinsic(const llvm::Function *function) {
  return DemangledName{ .fullName = function->getName(), .name = intrinsicName(function) };
}

const DemangledName &CPGDemangler::demangleFunctionName(const llvm::Function *function) {
  if (!cache.count(function)) {
    if (function->isIntrinsic()) {
      cache[function] = demangleIntrinsic(function);
    } else {
      std::string mangledName = function->getName();
      cache[function] = DemangledName{ .fullName = demangler.extractFullName(mangledName),
                                       .name = demangler.extractName(mangledName) };
    }
  }

  return cache.at(function);
}
