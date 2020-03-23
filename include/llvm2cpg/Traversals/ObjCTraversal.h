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

  bool operator==(const ObjCMethod &that) const {
    return this->name == that.name;
  }
};

struct ObjCMethodHash {
  std::size_t operator()(const ObjCMethod &m) const {
    return std::hash<std::string>{}(m.name);
  }
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

class ObjCCategoryDefinition {
public:
  ObjCCategoryDefinition(std::string categoryClassName, std::string categoryMetaclassName,
                         std::string name, ObjCClassDefinition *classDefinition,
                         const llvm::ConstantStruct *definition);
  std::string &getClassName();
  std::string &getMetaclassName();
  std::string &getName();
  ObjCClassDefinition *getClassDefinition();
  const llvm::ConstantStruct *getDefinition();

private:
  std::string categoryClassName;
  std::string categoryMetaclassName;
  std::string name;
  ObjCClassDefinition *classDefinition;
  const llvm::ConstantStruct *definition;
};

class ObjCTraversal {
public:
  explicit ObjCTraversal(const llvm::Module *bitcode);

  std::vector<ObjCClassDefinition *> objcClasses();
  std::vector<ObjCCategoryDefinition *> objcCategories();

  ObjCClassDefinition *objcSuperclass(ObjCClassDefinition *objcClass);
  ObjCClassDefinition *objcMetaclass(ObjCClassDefinition *objcClass);
  std::vector<ObjCMethod> objcClassMethods(ObjCClassDefinition *objcClass);
  std::vector<ObjCMethod> objcCategoryClassMethods(ObjCCategoryDefinition *objcCategory);
  std::vector<ObjCMethod> objcCategoryInstanceMethods(ObjCCategoryDefinition *objcCategory);

  ObjCClassDefinition *objcClassFromGlobalObject(const llvm::GlobalObject *global);
  ObjCCategoryDefinition *objcCategoryFromGlobalObject(const llvm::GlobalObject *global);

private:
  std::string objcClassName(const llvm::ConstantStruct *objcClass);
  std::string objcCategoryName(const llvm::ConstantStruct *objcCategory);

  const llvm::ConstantStruct *objcClassROCounterpart(const llvm::ConstantStruct *objcClass);
  bool shouldSkipGlobal(const llvm::GlobalObject *global);

  const llvm::Module *bitcode;
  const llvm::Type *class_t;
  const llvm::Type *class_ro_t;
  const llvm::Type *category_t;
  std::unordered_map<const llvm::GlobalObject *, ObjCClassDefinition *> classCache;
  std::vector<std::unique_ptr<ObjCClassDefinition>> ownedClassDefinitions;
  std::unordered_map<const llvm::GlobalObject *, ObjCCategoryDefinition *> categoryCache;
  std::vector<std::unique_ptr<ObjCCategoryDefinition>> ownedCategoryDefinitions;
};

} // namespace llvm2cpg
