#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace llvm {
class ConstantStruct;
class Module;
class Function;
class Type;
class GlobalObject;
} // namespace llvm

namespace llvm2cpg {

struct ObjCMethod {
  std::string name;
  llvm::Function *function;
};

class ObjCClassDefinition {
public:
  ObjCClassDefinition(std::string name, bool metaclass, const llvm::ConstantStruct *definition);
  std::string &getName();
  bool isExternal();
  bool isMetaclass();
  const llvm::ConstantStruct *getDefinition();

private:
  std::string name;
  bool metaclass;
  const llvm::ConstantStruct *definition;
};

class ObjCTraversal {
public:
  explicit ObjCTraversal(const llvm::Module *bitcode);

  std::vector<ObjCClassDefinition *> objcRootClasses();
  std::vector<ObjCClassDefinition *> objcClasses();
  std::vector<ObjCClassDefinition *> objcMetaclasses();

  ObjCClassDefinition *objcSuperclass(ObjCClassDefinition *objcClass);
  ObjCClassDefinition *objcMetaclass(ObjCClassDefinition *objcClass);

  std::vector<ObjCMethod> objcMethods(ObjCClassDefinition *objcClass);

private:
  std::string objcClassName(const llvm::ConstantStruct *objcClass);
  ObjCClassDefinition *objcClassFromGlobalObject(const llvm::GlobalObject *global);
  const llvm::ConstantStruct *objcClassROCounterpart(const llvm::ConstantStruct *objcClass);
  bool shouldSkipGlobal(const llvm::GlobalObject *global);

  const llvm::Module *bitcode;
  const llvm::Type *class_t;
  const llvm::Type *class_ro_t;
  std::unordered_map<const llvm::GlobalObject *, ObjCClassDefinition *> cache;
  std::vector<std::unique_ptr<ObjCClassDefinition>> ownedDefinitions;
};

} // namespace llvm2cpg
