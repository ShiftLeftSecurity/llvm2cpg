#pragma once

#include <string>
#include <vector>

namespace llvm {
class ConstantStruct;
class Module;
class Function;
} // namespace llvm

namespace llvm2cpg {

class ObjCTraversal {
public:
  std::vector<llvm::ConstantStruct *> objcClasses(llvm::Module &bitcode);
  std::string objcClassName(llvm::ConstantStruct *objcClass);
  std::vector<llvm::Function *> objcMethods(llvm::ConstantStruct *objcClass);

private:
};

} // namespace llvm2cpg
