#include "llvm2cpg/Traversals/ObjCTraversal.h"
#include <llvm/IR/Constants.h>
#include <llvm/IR/GlobalObject.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>

using namespace llvm2cpg;

ObjCTraversal::ObjCTraversal(const llvm::Module *bitcode)
    : bitcode(bitcode), class_t(bitcode->getTypeByName("struct._class_t")),
      class_ro_t(bitcode->getTypeByName("struct._class_ro_t")) {}

std::vector<const llvm::ConstantStruct *> ObjCTraversal::objcClasses() {
  std::vector<const llvm::ConstantStruct *> classes;
  if (!class_t || !class_ro_t) {
    return classes;
  }

  for (const llvm::GlobalObject &global : bitcode->global_objects()) {
    if (global.hasName() && global.getName().startswith("OBJC_CLASS_$")) {
      assert(global.getType() == class_t->getPointerTo(0));
      auto &variable = llvm::cast<llvm::GlobalVariable>(global);
      assert(variable.hasInitializer() && "ObjC class should be constant");
      auto *constantStruct = llvm::cast<llvm::ConstantStruct>(variable.getInitializer());
      classes.push_back(constantStruct);
    }
  }
  return classes;
}

std::string ObjCTraversal::objcClassName(const llvm::ConstantStruct *objcClass) {
  assert(objcClass);
  /// TODO: replace with type system
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

std::vector<std::pair<std::string, llvm::Function *>>
ObjCTraversal::objcMethods(const llvm::ConstantStruct *objcClass) {
  assert(objcClass->getType() == class_ro_t);
  llvm::Constant *methodsListSlot = objcClass->getAggregateElement(5);
  if (methodsListSlot->isNullValue()) {
    return std::vector<std::pair<std::string, llvm::Function *>>();
  }
  auto *methodsListSlotConstExpr =
      llvm::cast<llvm::ConstantExpr>(methodsListSlot)->getAsInstruction();
  auto *methodsListCast = llvm::cast<llvm::BitCastInst>(methodsListSlotConstExpr);
  auto *methodsListDecl = llvm::cast<llvm::GlobalVariable>(methodsListCast->getOperand(0));
  methodsListSlotConstExpr->deleteValue();
  assert(methodsListDecl->hasInitializer());
  auto *methodListStruct = llvm::cast<llvm::ConstantStruct>(methodsListDecl->getInitializer());
  auto *methodList = llvm::cast<llvm::ConstantArray>(methodListStruct->getAggregateElement(2));

  std::vector<std::pair<std::string, llvm::Function *>> methods;

  for (uint64_t i = 0; i < methodList->getType()->getNumElements(); i++) {
    llvm::Constant *methodStruct = methodList->getAggregateElement(i);

    auto *methodNameConstExpr =
        llvm::cast<llvm::ConstantExpr>(methodStruct->getAggregateElement(uint(0)));
    auto *methodNameGEP = llvm::cast<llvm::GetElementPtrInst>(methodNameConstExpr->getAsInstruction());
    auto *methodNameDecl = llvm::cast<llvm::GlobalVariable>(methodNameGEP->getOperand(0));
    assert(methodNameDecl->hasInitializer());
    auto *methodNameData = llvm::cast<llvm::ConstantDataArray>(methodNameDecl->getInitializer());
    std::string methodName = methodNameData->getAsCString();

    auto *methodRefConstExpr = llvm::cast<llvm::ConstantExpr>(methodStruct->getAggregateElement(2));
    auto *methodRefCast = llvm::cast<llvm::BitCastInst>(methodRefConstExpr->getAsInstruction());
    auto *method = llvm::cast<llvm::Function>(methodRefCast->getOperand(0));

    methodNameGEP->deleteValue();
    methodRefCast->deleteValue();

    methods.emplace_back(methodName, method);
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

static const llvm::ConstantStruct *extractClass(const llvm::ConstantStruct *objcClass, uint index) {
  const llvm::Constant *superclassSlot = objcClass->getAggregateElement(index);
  if (superclassSlot->isNullValue()) {
    return nullptr;
  }
  auto *superclassDecl = llvm::cast<const llvm::GlobalVariable>(superclassSlot);
  assert(superclassDecl->hasInitializer());

  auto *superclass = llvm::cast<const llvm::ConstantStruct>(superclassDecl->getInitializer());
  return superclass;
}

const llvm::ConstantStruct *ObjCTraversal::objcSuperclass(const llvm::ConstantStruct *objcClass) {
  assert(objcClass->getType() == class_t);
  return extractClass(objcClass, 1);
}

const llvm::ConstantStruct *ObjCTraversal::objcMetaclass(const llvm::ConstantStruct *objcClass) {
  assert(objcClass->getType() == class_t);
  return extractClass(objcClass, 0);
}
