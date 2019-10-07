#pragma once

#include "llvm2cpg/CPG/CPGFile.h"
#include "llvm2cpg/Transforms/Transforms.h"
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
  Transforms transforms;
  std::vector<CPGFile> files;
};

} // namespace llvm2cpg