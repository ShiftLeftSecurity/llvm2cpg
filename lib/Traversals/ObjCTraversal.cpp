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

ObjCTraversal::ObjCTraversal(const llvm::Module *bitcode)
    : bitcode(bitcode), class_t(findObjCClassTType(bitcode)),
      class_ro_t(findObjCClassROTType(bitcode)) {}

std::vector<ObjCClassDefinition *> ObjCTraversal::objcClasses() {
  std::vector<ObjCClassDefinition *> classes;
  if (!class_t || !class_ro_t) {
    return classes;
  }

  for (const llvm::GlobalObject &global : bitcode->global_objects()) {
    if (global.hasName() && global.getName().startswith(ObjCClassPrefix) &&
        !shouldSkipGlobal(&global)) {
      classes.push_back(objcClassFromGlobalObject(&global));
    }
  }
  return classes;
}

bool ObjCTraversal::shouldSkipGlobal(const llvm::GlobalObject *global) {
  /// There is at least one case when a `glue_class_t` is used instead of `class_t`: libarclite
  /// It is not clear if the glue_class_t is libarclite specific, or may appear in other contexts
  /// ignore for the time being
  /// TODO: check where the `glue_class_t` comes from
  return global->getType()->getPointerElementType() != class_t;
}

std::vector<ObjCClassDefinition *> ObjCTraversal::objcMetaclasses() {
  std::vector<ObjCClassDefinition *> classes;
  if (!class_t || !class_ro_t) {
    return classes;
  }

  for (const llvm::GlobalObject &global : bitcode->global_objects()) {
    if (global.hasName() && global.getName().startswith(ObjCMetaclassPrefix) &&
        !shouldSkipGlobal(&global)) {
      classes.push_back(objcClassFromGlobalObject(&global));
    }
  }
  return classes;
}

std::vector<ObjCClassDefinition *> ObjCTraversal::objcRootClasses() {
  std::vector<ObjCClassDefinition *> classes;
  if (!class_t || !class_ro_t) {
    return classes;
  }

  for (const llvm::GlobalObject &global : bitcode->global_objects()) {
    llvm::LLVMContext &context = bitcode->getContext();
    unsigned rootClassMD = context.getMDKindID("shiftleft.objc_root_class");
    if (global.hasMetadata(rootClassMD) && !shouldSkipGlobal(&global)) {
      classes.push_back(objcClassFromGlobalObject(&global));
    }
  }
  return classes;
}

std::string ObjCTraversal::objcClassName(const llvm::ConstantStruct *objcClass) {
  assert(objcClass);
  assert(objcClass->getType() == class_ro_t);
  llvm::Constant *classNameSlot = objcClass->getAggregateElement(4);

  auto *classNameConstExpr = llvm::cast<llvm::ConstantExpr>(classNameSlot)->getAsInstruction();
  auto *classNameRef = llvm::cast<llvm::GetElementPtrInst>(classNameConstExpr);
  assert(classNameRef->getNumIndices() == 2);
  assert(classNameRef->getNumOperands() == 3);
  assert(llvm::cast<llvm::ConstantInt>(classNameRef->getOperand(1))->isZero());
  assert(llvm::cast<llvm::ConstantInt>(classNameRef->getOperand(2))->isZero());

  auto *classNameDecl = llvm::cast<llvm::GlobalVariable>(classNameRef->getOperand(0));
  assert(classNameDecl->hasInitializer() && "ObjC classes should have name");
  classNameConstExpr->deleteValue();

  auto *classNameData = llvm::cast<llvm::ConstantDataArray>(classNameDecl->getInitializer());
  return classNameData->getAsCString();
}

std::vector<ObjCMethod> ObjCTraversal::objcMethods(ObjCClassDefinition *objcClass) {
  assert(objcClass);
  if (objcClass->isExternal()) {
    return std::vector<ObjCMethod>();
  }
  const llvm::ConstantStruct *objcClassRO = objcClassROCounterpart(objcClass->getDefinition());
  llvm::Constant *methodsListSlot = objcClassRO->getAggregateElement(5);
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
  return objcClassFromGlobalObject(llvm::cast<llvm::GlobalObject>(global));
}

ObjCClassDefinition *ObjCTraversal::objcMetaclass(ObjCClassDefinition *objcClass) {
  assert(objcClass);
  if (objcClass->isExternal()) {
    return nullptr;
  }

  llvm::Constant *global = objcClass->getDefinition()->getAggregateElement(unsigned(0));
  return objcClassFromGlobalObject(llvm::cast<llvm::GlobalObject>(global));
}

ObjCClassDefinition *ObjCTraversal::objcClassFromGlobalObject(const llvm::GlobalObject *global) {
  assert(global);
  assert(global->hasName());
  assert(global->getName().startswith(ObjCClassPrefix) ||
         global->getName().startswith(ObjCMetaclassPrefix));

  if (cache.count(global)) {
    return cache.at(global);
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

  ownedDefinitions.emplace_back(new ObjCClassDefinition(className, metaclass, definition));
  cache[global] = ownedDefinitions.back().get();

  return ownedDefinitions.back().get();
}
