#pragma once

#include "llvm2cpg/Traversals/ObjCTraversal.h"
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace llvm {
class ConstantStruct;
class Module;
class Function;
} // namespace llvm

namespace llvm2cpg {

class CPGLogger;

class ObjCTypeHierarchy {
public:
  ObjCTypeHierarchy(CPGLogger &logger, std::vector<const llvm::Module *> &modules);
  std::vector<std::string> getRootClasses();
  std::vector<std::string> getClasses();
  std::vector<std::string> getSubclasses(const std::string &objcClass);
  std::string getMetaclass(const std::string &objcClass);
  std::vector<ObjCMethod> getMethods(const std::string &objcClass);

  bool isExternal(const std::string &name) const;

  void propagateSubclassMethods();

private:
  void constructHierarchy(std::vector<const llvm::Module *> &modules);

  void fillInMethods(const std::string &base, const std::string &derived);

  CPGLogger &logger;

  std::unordered_set<std::string> rootClasses;
  std::unordered_set<std::string> objcClasses;
  std::unordered_set<std::string> definedClasses;

  std::unordered_map<std::string, std::string> metaclassMapping;
  std::unordered_map<std::string, std::unordered_set<std::string>> subclassMapping;
  std::unordered_map<std::string, std::unordered_set<ObjCMethod, ObjCMethodHash>> methodMapping;
};

} // namespace llvm2cpg