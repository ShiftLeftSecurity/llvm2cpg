#pragma once

namespace llvm {
class Module;
class Function;
}

namespace llvm2cpg {

class Transforms {
public:
  void transformBitcode(llvm::Module &bitcode);

private:
  void destructPHINodes(llvm::Function &function);
  void renameOpaqueObjCTypes(llvm::Module &bitcode);
};

} // namespace llvm2cpg