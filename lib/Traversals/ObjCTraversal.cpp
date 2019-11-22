#include "llvm2cpg/Traversals/ObjCTraversal.h"
#include <llvm/IR/Constants.h>
#include <llvm/IR/GlobalObject.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>

using namespace llvm2cpg;

std::vector<llvm::ConstantStruct *> ObjCTraversal::objcClasses(llvm::Module &bitcode) {
  llvm::Type *class_ro_t = bitcode.getTypeByName("struct._class_ro_t");
  if (!class_ro_t) {
    return std::vector<llvm::ConstantStruct *>();
  }

  std::vector<llvm::ConstantStruct *> classes;
  for (llvm::GlobalObject &global : bitcode.global_objects()) {
    if (global.hasName() && global.getName().startswith("_OBJC_CLASS_RO_$")) {
      auto &variable = llvm::cast<llvm::GlobalVariable>(global);
      assert(variable.hasInitializer() && "ObjC class should be constant");
      auto *constantStruct = llvm::cast<llvm::ConstantStruct>(variable.getInitializer());
      classes.push_back(constantStruct);
    }
  }
  return classes;
}

std::string ObjCTraversal::objcClassName(llvm::ConstantStruct *objcClass) {
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

std::vector<llvm::Function *> ObjCTraversal::objcMethods(llvm::ConstantStruct *objcClass) {
  llvm::Constant *methodsListSlot = objcClass->getAggregateElement(5);
  auto *methodsListSlotConstExpr =
      llvm::cast<llvm::ConstantExpr>(methodsListSlot)->getAsInstruction();
  auto *methodsListCast = llvm::cast<llvm::BitCastInst>(methodsListSlotConstExpr);
  auto *methodsListDecl = llvm::cast<llvm::GlobalVariable>(methodsListCast->getOperand(0));
  methodsListSlotConstExpr->deleteValue();
  assert(methodsListDecl->hasInitializer());
  auto *methodListStruct = llvm::cast<llvm::ConstantStruct>(methodsListDecl->getInitializer());
  auto *methodList = llvm::cast<llvm::ConstantArray>(methodListStruct->getAggregateElement(2));

  std::vector<llvm::Function *> methods;

  for (uint64_t i = 0; i < methodList->getType()->getNumElements(); i++) {
    llvm::Constant *methodStruct = methodList->getAggregateElement(i);
    auto *methodRefConstExpr = llvm::cast<llvm::ConstantExpr>(methodStruct->getAggregateElement(2));
    auto *methodRefCast = llvm::cast<llvm::BitCastInst>(methodRefConstExpr->getAsInstruction());
    auto *method = llvm::cast<llvm::Function>(methodRefCast->getOperand(0));
    methods.push_back(method);
    methodRefCast->deleteValue();
  }

  return methods;
}
