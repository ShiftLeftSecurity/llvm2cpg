#pragma once

namespace llvm {
class Module;
}

namespace llvm2cpg {

class Transforms {
public:
  void transformBitcode(llvm::Module &bitcode);

private:
};

} // namespace llvm2cpg