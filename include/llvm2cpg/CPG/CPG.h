#pragma once

#include "CPGFile.h"
#include <vector>

namespace llvm {
class Module;
}

namespace llvm2cpg {

class CPG {
public:
  const std::vector<CPGFile> &getFiles() const;
  void addBitcode(llvm::Module *bitcode);

private:
  std::vector<CPGFile> files;
};

} // namespace llvm2cpg