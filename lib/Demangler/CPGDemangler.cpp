#include "llvm2cpg/Demangler/CPGDemangler.h"
#include <llvm/IR/Function.h>

using namespace llvm2cpg;

const DemangledName &CPGDemangler::demangleFunctionName(const llvm::Function *function) {
  if (!cache.count(function)) {
    std::string mangledName = function->getName();
    cache[function] = DemangledName{ .fullName = demangler.extractFullName(mangledName),
                                     .name = demangler.extractName(mangledName) };
  }

  return cache.at(function);
}
