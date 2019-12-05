#include "llvm2cpg/CPG/ObjCTypeHierarchy.h"

#include <llvm/IR/Function.h>
#include <llvm/Support/raw_ostream.h>
#include <utility>

using namespace llvm2cpg;

ObjCTypeHierarchy::ObjCTypeHierarchy(const llvm::Module *module) : traversal(module) {
  constructHierarchy();
}

std::unordered_set<ObjCClassDefinition *> &ObjCTypeHierarchy::getRootClasses() {
  return rootClasses;
}

std::unordered_set<ObjCClassDefinition *> &ObjCTypeHierarchy::getClasses() {
  return objcClasses;
}

std::vector<ObjCClassDefinition *> &
ObjCTypeHierarchy::getSubclasses(ObjCClassDefinition *objcClass) {
  return subclasses[objcClass];
}

ObjCClassDefinition *ObjCTypeHierarchy::getMetaclass(ObjCClassDefinition *objcClass) {
  return traversal.objcMetaclass(objcClass);
}

std::vector<ObjCMethod> &ObjCTypeHierarchy::getMethods(ObjCClassDefinition *objcClass) {
  return methodMapping[objcClass];
}

void ObjCTypeHierarchy::constructHierarchy() {
  std::vector<ObjCClassDefinition *> worklist;

  for (ObjCClassDefinition *root : traversal.objcRootClasses()) {
    rootClasses.insert(root);
  }

  for (ObjCClassDefinition *objcClass : traversal.objcClasses()) {
    recordObjCClass(objcClass);
    worklist.push_back(objcClass);
  }

  for (ObjCClassDefinition *objcMetaclass : traversal.objcMetaclasses()) {
    recordObjCClass(objcMetaclass);
    worklist.push_back(objcMetaclass);
  }

  for (ObjCClassDefinition *objcClass : worklist) {
    assert(objcClass);
    if (!rootClasses.count(objcClass)) {
      ObjCClassDefinition *superclass = traversal.objcSuperclass(objcClass);
      assert(superclass);
      inherit(superclass, objcClass);
    }
  }

  for (ObjCClassDefinition *objcClass : worklist) {
    methodMapping[objcClass] = traversal.objcMethods(objcClass);
  }
}

void ObjCTypeHierarchy::recordObjCClass(ObjCClassDefinition *objcClass) {
  assert(objcClass);
  if (objcClass->isMetaclass()) {
    objcMetaclasses.insert(objcClass);
  } else {
    objcClasses.insert(objcClass);
  }
  superclasses[objcClass] = nullptr;
  subclasses[objcClass] = std::vector<ObjCClassDefinition *>();
  methodMapping[objcClass] = std::vector<ObjCMethod>();
}

void ObjCTypeHierarchy::inherit(ObjCClassDefinition *base, ObjCClassDefinition *derived) {
  assert(base);
  assert(derived);
  superclasses[derived] = base;
  subclasses.at(base).push_back(derived);
}

void ObjCTypeHierarchy::fillInMethods(ObjCClassDefinition *base, ObjCClassDefinition *derived) {
  std::unordered_map<std::string, ObjCMethod> methods;
  for (ObjCMethod &method : methodMapping[base]) {
    methods[method.name] = method;
  }
  for (const auto &method : traversal.objcMethods(derived)) {
    methods[method.name] = method;
  }

  std::vector<ObjCMethod> derivedMethods;
  derivedMethods.reserve(methods.size());
  for (const auto &pair : methods) {
    derivedMethods.push_back(pair.second);
  }
  methodMapping[derived].swap(derivedMethods);
  for (ObjCClassDefinition *derivedClass : getSubclasses(derived)) {
    fillInMethods(derived, derivedClass);
  }
}

void ObjCTypeHierarchy::propagateSubclassMethods() {
  for (ObjCClassDefinition *baseClass : rootClasses) {
    for (ObjCClassDefinition *derivedClass : getSubclasses(baseClass)) {
      fillInMethods(baseClass, derivedClass);
    }
  }
}
