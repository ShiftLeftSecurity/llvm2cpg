#include "llvm2cpg/CPG/ObjCTypeHierarchy.h"
#include "llvm2cpg/Logger/CPGLogger.h"
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/Support/raw_ostream.h>
#include <sstream>

using namespace llvm2cpg;

ObjCTypeHierarchy::ObjCTypeHierarchy(llvm2cpg::CPGLogger &logger,
                                     std::vector<const llvm::Module *> &modules)
    : logger(logger) {
  constructHierarchy(modules);
}

std::vector<std::string> ObjCTypeHierarchy::getRootClasses() {
  std::vector<std::string> result;
  for (auto &pair : rootClasses) {
    result.push_back(pair);
  }
  return result;
}

std::vector<std::string> ObjCTypeHierarchy::getClasses() {
  std::vector<std::string> result;
  for (auto &pair : objcClasses) {
    result.push_back(pair);
  }
  return result;
}

std::vector<std::string> ObjCTypeHierarchy::getCategories() {
  std::vector<std::string> result;
  for (auto &pair : objcCategories) {
    result.push_back(pair);
  }
  return result;
}

std::vector<std::string> ObjCTypeHierarchy::getSubclasses(const std::string &objcClass) {
  std::vector<std::string> result;
  for (auto &pair : subclassMapping[objcClass]) {
    result.push_back(pair);
  }
  return result;
}

std::string ObjCTypeHierarchy::getMetaclass(const std::string &objcClass) {
  return metaclassMapping[objcClass];
}

std::vector<ObjCMethod> ObjCTypeHierarchy::getMethods(const std::string &objcClass) {
  std::vector<ObjCMethod> result;
  for (auto &pair : methodMapping[objcClass]) {
    result.push_back(pair);
  }
  return result;
}

void ObjCTypeHierarchy::constructHierarchy(std::vector<const llvm::Module *> &modules) {
  for (const llvm::Module *module : modules) {
    ObjCTraversal traversal(module);
    std::vector<ObjCClassDefinition *> worklist;
    for (ObjCClassDefinition *objcClass : traversal.objcClasses()) {
      worklist.push_back(objcClass);
    }

    /// TODO: describe how it works
    for (ObjCClassDefinition *objcClass : worklist) {
      ObjCClassDefinition *metaClass = traversal.objcMetaclass(objcClass);
      assert(metaClass);

      ObjCClassDefinition *superClass = traversal.objcSuperclass(objcClass);
      ObjCClassDefinition *metaSuperclass = traversal.objcSuperclass(metaClass);

      metaclassMapping[objcClass->getName()] = metaClass->getName();
      if (superClass && metaSuperclass) {
        metaclassMapping[superClass->getName()] = metaSuperclass->getName();
      }

      if (superClass && metaSuperclass) {
        subclassMapping[superClass->getName()].insert(objcClass->getName());
        subclassMapping[metaSuperclass->getName()].insert(metaClass->getName());
        if (superClass->isExternal()) {
          rootClasses.insert(superClass->getName());
        }
        if (metaSuperclass->isExternal()) {
          rootClasses.insert(metaSuperclass->getName());
        }
      } else {
        rootClasses.insert(objcClass->getName());
        rootClasses.insert(metaClass->getName());
      }

      definedClasses.insert(objcClass->getName());
      definedClasses.insert(metaClass->getName());

      objcClasses.insert(objcClass->getName());
      if (superClass) {
        objcClasses.insert(superClass->getName());
      }

      for (ObjCMethod &method : traversal.objcClassMethods(objcClass)) {
        methodMapping[objcClass->getName()].insert(method);
      }
      for (ObjCMethod &method : traversal.objcClassMethods(metaClass)) {
        methodMapping[metaClass->getName()].insert(method);
      }
    }
    for (ObjCCategoryDefinition *category : traversal.objcCategories()) {
      std::string categoryName = category->getClassName() + "(" + category->getName() + ")";
      objcClasses.insert(category->getClassName());
      if (category->getClassDefinition()->isExternal()) {
        rootClasses.insert(category->getClassName());
        rootClasses.insert(category->getMetaclassName());
        metaclassMapping[category->getClassName()] = category->getMetaclassName();
      }
      objcCategories.insert(categoryName);
      for (ObjCMethod &method : traversal.objcCategoryInstanceMethods(category)) {
        methodMapping[category->getClassName()].insert(method);
        methodMapping[categoryName].insert(method);
      }
      for (ObjCMethod &method : traversal.objcCategoryClassMethods(category)) {
        methodMapping[category->getMetaclassName()].insert(method);
        methodMapping[categoryName].insert(method);
      }
    }
  }
  logger.doNothing();
}

void ObjCTypeHierarchy::fillInMethods(const std::string &base, const std::string &derived) {
  std::unordered_map<std::string, ObjCMethod> methods;
  for (auto &method : methodMapping[base]) {
    methods[method.name] = method;
  }
  for (auto &method : methodMapping[derived]) {
    methods[method.name] = method;
  }

  std::unordered_set<ObjCMethod, ObjCMethodHash> derivedMethods;
  derivedMethods.reserve(methods.size());
  for (const auto &pair : methods) {
    derivedMethods.insert(pair.second);
  }
  methodMapping[derived].swap(derivedMethods);

  for (const std::string &derivedClass : subclassMapping[derived]) {
    fillInMethods(derived, derivedClass);
  }
}

void ObjCTypeHierarchy::propagateSubclassMethods() {
  for (const std::string &base : rootClasses) {
    for (const std::string &derived : subclassMapping[base]) {
      fillInMethods(base, derived);
    }
  }
}

bool ObjCTypeHierarchy::isExternal(const std::string &name) const {
  return definedClasses.count(name) == 0;
}
