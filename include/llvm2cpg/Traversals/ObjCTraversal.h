#pragma once

#include <string>
#include <vector>

namespace llvm {
class ConstantStruct;
class Module;
class Function;
class Type;
} // namespace llvm

namespace llvm2cpg {

class ObjCTraversal {
public:
  explicit ObjCTraversal(const llvm::Module *bitcode);
  std::vector<const llvm::ConstantStruct *> objcClasses();
  const llvm::ConstantStruct *objcClassROCounterpart(const llvm::ConstantStruct *objcClass);
  const llvm::ConstantStruct *objcSuperclass(const llvm::ConstantStruct *objcClass);
  std::string objcClassName(const llvm::ConstantStruct *objcClass);
  std::vector<llvm::Function *> objcMethods(const llvm::ConstantStruct *objcClass);

private:
  const llvm::Module *bitcode;
  const llvm::Type *class_t;
  const llvm::Type *class_ro_t;
};

} // namespace llvm2cpg
