#pragma once

#include "llvm2cpg/Demangler/Demangler.h"
#include <string>
#include <unordered_map>

namespace llvm {
class Function;
}

namespace llvm2cpg {

struct DemangledName {
  std::string fullName;
  std::string name;
};

class CPGDemangler {
public:
  const DemangledName &demangleFunctionName(const llvm::Function *function);

private:
  Demangler demangler;
  std::unordered_map<const llvm::Function *, DemangledName> cache;
};

}