#include "llvm2cpg/Traversals/ObjCTraversal.h"
#include <llvm/IR/Constants.h>
#include <llvm/IR/GlobalObject.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>

#include <utility>

static const char *ObjCClassPrefix = "OBJC_CLASS_$_";
static const char *ObjCMetaclassPrefix = "OBJC_METACLASS_$_";

using namespace llvm2cpg;

static std::string className(std::string name, bool metaclass) {
  assert(!name.empty());
  if (metaclass) {
    return name + "$";
  }
  return name;
}

ObjCClassDefinition::ObjCClassDefinition(std::string name, bool metaclass,
                                         const llvm::ConstantStruct *definition)
    : name(className(std::move(name), metaclass)), metaclass(metaclass), definition(definition) {}

std::string &ObjCClassDefinition::getName() {
  return name;
}

bool ObjCClassDefinition::isExternal() {
  return definition == nullptr;
}

bool ObjCClassDefinition::isMetaclass() {
  return metaclass;
}

const llvm::ConstantStruct *ObjCClassDefinition::getDefinition() {
  return definition;
}

/// ObjCCategoryDefinition

ObjCCategoryDefinition::ObjCCategoryDefinition(std::string categoryClassName,
                                               std::string categoryMetaclassName, std::string name,
                                               ObjCClassDefinition *classDefinition,
                                               const llvm::ConstantStruct *definition)
    : categoryClassName(std::move(categoryClassName)),
      categoryMetaclassName(std::move(categoryMetaclassName)), name(std::move(name)),
      classDefinition(classDefinition), definition(definition) {}

std::string &ObjCCategoryDefinition::getClassName() {
  return categoryClassName;
}

std::string &ObjCCategoryDefinition::getMetaclassName() {
  return categoryMetaclassName;
}

std::string &ObjCCategoryDefinition::getName() {
  return name;
}

const llvm::ConstantStruct *ObjCCategoryDefinition::getDefinition() {
  return definition;
}

ObjCClassDefinition *ObjCCategoryDefinition::getClassDefinition() {
  return classDefinition;
}

/// Traversals

static llvm::Type *findObjCClassType(const llvm::Module *bitcode, const std::string &typePrefix) {
  for (llvm::StructType *type : bitcode->getIdentifiedStructTypes()) {
    if (type->hasName() && type->getName().startswith(typePrefix)) {
      return type;
    }
  }
  return nullptr;
}

static llvm::Type *findObjCClassTType(const llvm::Module *bitcode) {
  return findObjCClassType(bitcode, "struct._class_t");
}

static llvm::Type *findObjCClassROTType(const llvm::Module *bitcode) {
  return findObjCClassType(bitcode, "struct._class_ro_t");
}

static llvm::Type *findObjCCategoryType(const llvm::Module *bitcode) {
  return findObjCClassType(bitcode, "struct._category_t");
}

ObjCTraversal::ObjCTraversal(const llvm::Module *bitcode)
    : bitcode(bitcode), class_t(findObjCClassTType(bitcode)),
      class_ro_t(findObjCClassROTType(bitcode)), category_t(findObjCCategoryType(bitcode)) {}

std::vector<ObjCClassDefinition *> ObjCTraversal::objcClasses() {
  std::vector<ObjCClassDefinition *> classes;
  if (!class_t || !class_ro_t) {
    return classes;
  }
  const llvm::GlobalVariable *objcClassList =
      bitcode->getGlobalVariable("OBJC_LABEL_CLASS_$", true);
  if (!objcClassList) {
    return classes;
  }
  assert(objcClassList->hasInitializer());
  const auto *objcClasses = llvm::cast<llvm::ConstantArray>(objcClassList->getInitializer());
  for (unsigned i = 0; i < objcClasses->getNumOperands(); i++) {
    assert(llvm::isa<llvm::ConstantExpr>(objcClasses->getOperand(i)));
    const auto *bitcast = llvm::cast<llvm::ConstantExpr>(objcClasses->getOperand(i));
    assert(llvm::isa<llvm::GlobalVariable>(bitcast->getOperand(0)));
    const auto *global = llvm::cast<llvm::GlobalVariable>(bitcast->getOperand(0));
    if (!shouldSkipGlobal(global)) {
      classes.push_back(objcClassFromGlobalObject(global));
    }
  }
  return classes;
}

std::vector<ObjCCategoryDefinition *> ObjCTraversal::objcCategories() {
  std::vector<ObjCCategoryDefinition *> categories;
  if (!category_t) {
    return categories;
  }
  const llvm::GlobalVariable *categoryList =
      bitcode->getGlobalVariable("OBJC_LABEL_CATEGORY_$", true);
  if (!categoryList) {
    return categories;
  }
  assert(categoryList->hasInitializer());
  const auto *objcClasses = llvm::cast<llvm::ConstantArray>(categoryList->getInitializer());
  for (unsigned i = 0; i < objcClasses->getNumOperands(); i++) {
    assert(llvm::isa<llvm::ConstantExpr>(objcClasses->getOperand(i)));
    const auto *bitcast = llvm::cast<llvm::ConstantExpr>(objcClasses->getOperand(i));
    assert(llvm::isa<llvm::GlobalVariable>(bitcast->getOperand(0)));
    const auto *global = llvm::cast<llvm::GlobalVariable>(bitcast->getOperand(0));
    categories.push_back(objcCategoryFromGlobalObject(global));
  }
  return categories;
}

bool ObjCTraversal::shouldSkipGlobal(const llvm::GlobalObject *global) {
  /// There is at least one case when a `glue_class_t` is used instead of `class_t`: libarclite
  /// It is not clear if the glue_class_t is libarclite specific, or may appear in other contexts
  /// ignore for the time being
  /// TODO: check where the `glue_class_t` comes from
  return global->getType()->getPointerElementType() != class_t;
}

static std::string extractName(const llvm::ConstantStruct *objcObject, unsigned slot) {
  llvm::Constant *nameSlot = objcObject->getAggregateElement(slot);

  auto *nameConstExpr = llvm::cast<llvm::ConstantExpr>(nameSlot)->getAsInstruction();
  auto *nameRef = llvm::cast<llvm::GetElementPtrInst>(nameConstExpr);
  assert(nameRef->getNumIndices() == 2);
  assert(nameRef->getNumOperands() == 3);
  assert(llvm::cast<llvm::ConstantInt>(nameRef->getOperand(1))->isZero());
  assert(llvm::cast<llvm::ConstantInt>(nameRef->getOperand(2))->isZero());

  auto *nameDecl = llvm::cast<llvm::GlobalVariable>(nameRef->getOperand(0));
  assert(nameDecl->hasInitializer() && "ObjC classes/categories should have name");
  nameConstExpr->deleteValue();

  auto *nameData = llvm::cast<llvm::ConstantDataArray>(nameDecl->getInitializer());
  return nameData->getAsCString();
}

std::string ObjCTraversal::objcClassName(const llvm::ConstantStruct *objcClass) {
  assert(objcClass);
  assert(objcClass->getType() == class_ro_t);
  return extractName(objcClass, 4);
}

std::string ObjCTraversal::objcCategoryName(const llvm::ConstantStruct *objcCategory) {
  assert(objcCategory);
  assert(objcCategory->getType() == category_t);
  return extractName(objcCategory, 0);
}

static std::vector<ObjCMethod> objcMethods(llvm::Constant *methodsListSlot) {
  if (methodsListSlot->isNullValue()) {
    return std::vector<ObjCMethod>();
  }
  auto *methodsListSlotConstExpr =
      llvm::cast<llvm::ConstantExpr>(methodsListSlot)->getAsInstruction();
  auto *methodsListCast = llvm::cast<llvm::BitCastInst>(methodsListSlotConstExpr);
  auto *methodsListDecl = llvm::cast<llvm::GlobalVariable>(methodsListCast->getOperand(0));
  methodsListSlotConstExpr->deleteValue();
  assert(methodsListDecl->hasInitializer());
  auto *methodListStruct = llvm::cast<llvm::ConstantStruct>(methodsListDecl->getInitializer());
  auto *methodList = llvm::cast<llvm::ConstantArray>(methodListStruct->getAggregateElement(2));

  std::vector<ObjCMethod> methods;

  for (uint64_t i = 0; i < methodList->getType()->getNumElements(); i++) {
    llvm::Constant *methodStruct = methodList->getAggregateElement(i);

    auto *methodNameConstExpr =
        llvm::cast<llvm::ConstantExpr>(methodStruct->getAggregateElement(uint(0)));
    auto *methodNameGEP =
        llvm::cast<llvm::GetElementPtrInst>(methodNameConstExpr->getAsInstruction());
    auto *methodNameDecl = llvm::cast<llvm::GlobalVariable>(methodNameGEP->getOperand(0));
    assert(methodNameDecl->hasInitializer());
    auto *methodNameData = llvm::cast<llvm::ConstantDataArray>(methodNameDecl->getInitializer());
    std::string methodName = methodNameData->getAsCString();

    auto *methodRefConstExpr = llvm::cast<llvm::ConstantExpr>(methodStruct->getAggregateElement(2));
    auto *methodRefCast = llvm::cast<llvm::BitCastInst>(methodRefConstExpr->getAsInstruction());
    auto *method = llvm::cast<llvm::Function>(methodRefCast->getOperand(0));

    methodNameGEP->deleteValue();
    methodRefCast->deleteValue();

    methods.emplace_back(ObjCMethod{ methodName, method });
  }

  return methods;
}

std::vector<ObjCMethod> ObjCTraversal::objcClassMethods(ObjCClassDefinition *objcClass) {
  assert(objcClass);
  if (objcClass->isExternal()) {
    return std::vector<ObjCMethod>();
  }
  const llvm::ConstantStruct *objcClassRO = objcClassROCounterpart(objcClass->getDefinition());
  return objcMethods(objcClassRO->getAggregateElement(5));
}

std::vector<ObjCMethod>
ObjCTraversal::objcCategoryClassMethods(llvm2cpg::ObjCCategoryDefinition *objcCategory) {
  assert(objcCategory);
  assert(objcCategory->getDefinition());
  return objcMethods(objcCategory->getDefinition()->getAggregateElement(3));
}

std::vector<ObjCMethod>
ObjCTraversal::objcCategoryInstanceMethods(llvm2cpg::ObjCCategoryDefinition *objcCategory) {
  assert(objcCategory);
  assert(objcCategory->getDefinition());
  return objcMethods(objcCategory->getDefinition()->getAggregateElement(2));
}

const llvm::ConstantStruct *
ObjCTraversal::objcClassROCounterpart(const llvm::ConstantStruct *objcClass) {
  assert(objcClass->getType() == class_t);

  const llvm::Constant *objcClassROSlot =
      objcClass->getAggregateElement(objcClass->getType()->getNumElements() - 1);
  auto *objcClassRODecl = llvm::cast<const llvm::GlobalVariable>(objcClassROSlot);
  assert(objcClassRODecl->hasInitializer());
  auto *objcClassRO = llvm::cast<const llvm::ConstantStruct>(objcClassRODecl->getInitializer());
  return objcClassRO;
}

ObjCClassDefinition *ObjCTraversal::objcSuperclass(ObjCClassDefinition *objcClass) {
  assert(objcClass);
  if (objcClass->isExternal()) {
    return nullptr;
  }
  llvm::Constant *global = objcClass->getDefinition()->getAggregateElement(unsigned(1));
  assert(global);
  if (global->isNullValue()) {
    return nullptr;
  }
  return objcClassFromGlobalObject(llvm::cast<llvm::GlobalObject>(global));
}

ObjCClassDefinition *ObjCTraversal::objcMetaclass(ObjCClassDefinition *objcClass) {
  assert(objcClass);
  if (objcClass->isExternal()) {
    return nullptr;
  }
  llvm::Constant *global = objcClass->getDefinition()->getAggregateElement(unsigned(0));
  assert(global);
  if (global->isNullValue()) {
    return nullptr;
  }
  return objcClassFromGlobalObject(llvm::cast<llvm::GlobalObject>(global));
}

ObjCClassDefinition *ObjCTraversal::objcClassFromGlobalObject(const llvm::GlobalObject *global) {
  assert(global);
  assert(global->hasName());
  assert(global->getName().startswith(ObjCClassPrefix) ||
         global->getName().startswith(ObjCMetaclassPrefix));

  if (classCache.count(global)) {
    return classCache.at(global);
  }

  const llvm::ConstantStruct *definition = nullptr;
  bool metaclass = global->getName().startswith(ObjCMetaclassPrefix);
  std::string className;
  auto *variable = llvm::cast<llvm::GlobalVariable>(global);
  if (variable->hasInitializer()) {
    definition = llvm::cast<const llvm::ConstantStruct>(variable->getInitializer());
    auto *definitionRO = objcClassROCounterpart(definition);
    className = objcClassName(definitionRO);
  } else {
    size_t prefix = metaclass ? strlen(ObjCMetaclassPrefix) : strlen(ObjCClassPrefix);
    className = variable->getName().substr(prefix);
  }

  ownedClassDefinitions.emplace_back(new ObjCClassDefinition(className, metaclass, definition));
  classCache[global] = ownedClassDefinitions.back().get();

  return ownedClassDefinitions.back().get();
}

ObjCCategoryDefinition *
ObjCTraversal::objcCategoryFromGlobalObject(const llvm::GlobalObject *global) {
  assert(global);
  assert(global->hasName());

  if (categoryCache.count(global)) {
    return categoryCache.at(global);
  }

  auto *variable = llvm::cast<llvm::GlobalVariable>(global);
  const auto *definition = llvm::cast<const llvm::ConstantStruct>(variable->getInitializer());
  const auto *classDefinition =
      llvm::cast<llvm::GlobalObject>(definition->getAggregateElement(unsigned(1)));
  ObjCClassDefinition *objcClass = objcClassFromGlobalObject(classDefinition);
  ownedCategoryDefinitions.emplace_back(
      new ObjCCategoryDefinition(className(objcClass->getName(), false),
                                 className(objcClass->getName(), true),
                                 objcCategoryName(definition),
                                 objcClass,
                                 definition));
  categoryCache[global] = ownedCategoryDefinitions.back().get();

  return ownedCategoryDefinitions.back().get();
}
