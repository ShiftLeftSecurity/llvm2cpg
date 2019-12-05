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

class ObjCTypeHierarchy {
public:
  explicit ObjCTypeHierarchy(const llvm::Module *module);
  std::unordered_set<ObjCClassDefinition *> &getRootClasses();
  std::unordered_set<ObjCClassDefinition *> &getClasses();

  ObjCClassDefinition *getMetaclass(ObjCClassDefinition *objcClass);
  std::vector<ObjCClassDefinition *> &getSubclasses(ObjCClassDefinition *objcClass);
  void propagateSubclassMethods();

  std::vector<ObjCMethod> &getMethods(ObjCClassDefinition *objcClass);

private:
  void constructHierarchy();
  void inherit(ObjCClassDefinition *base, ObjCClassDefinition *derived);
  void fillInMethods(ObjCClassDefinition *base, ObjCClassDefinition *derived);
  void recordObjCClass(ObjCClassDefinition *objcClass);

  ObjCTraversal traversal;

  std::unordered_set<ObjCClassDefinition *> rootClasses;
  std::unordered_set<ObjCClassDefinition *> objcClasses;
  std::unordered_set<ObjCClassDefinition *> objcMetaclasses;

  std::unordered_map<ObjCClassDefinition *, ObjCClassDefinition *> superclasses;
  std::unordered_map<ObjCClassDefinition *, std::vector<ObjCClassDefinition *>> subclasses;

  std::unordered_map<ObjCClassDefinition *, std::vector<ObjCMethod>> methodMapping;
};

} // namespace llvm2cpg