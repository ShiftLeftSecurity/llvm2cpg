#include "llvm2cpg/Transforms/Transforms.h"
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/Transforms/Utils/Local.h>
#include <llvm2cpg/Traversals/ObjCTraversal.h>

using namespace llvm2cpg;

void Transforms::transformBitcode(llvm::Module &bitcode) {
  renameOpaqueObjCTypes(bitcode);

  for (llvm::Function &function : bitcode) {
    if (function.isDeclaration()) {
      continue;
    }
    destructPHINodes(function);
  }
}

void Transforms::destructPHINodes(llvm::Function &function) {
  std::vector<llvm::PHINode *> worklist;
  for (llvm::Instruction &instruction : llvm::instructions(function)) {
    if (auto phi = llvm::dyn_cast<llvm::PHINode>(&instruction)) {
      worklist.push_back(phi);
    }
  }

  for (llvm::PHINode *phi : worklist) {
    llvm::DemotePHIToStack(phi);
  }
}

void Transforms::renameOpaqueObjCTypes(llvm::Module &bitcode) {
  ObjCTraversal traversal(&bitcode);
  std::vector<const llvm::ConstantStruct *> worklist = traversal.objcClasses();

  for (const llvm::ConstantStruct *objcClass : worklist) {
    const llvm::ConstantStruct *objcROClass = traversal.objcClassROCounterpart(objcClass);
    std::string className = traversal.objcClassName(objcROClass);

    std::vector<std::pair<std::string, llvm::Function *>> methods =
        traversal.objcMethods(objcROClass);
    for (auto &methodPair : methods) {
      llvm::FunctionType *type = methodPair.second->getFunctionType();
      assert(type->getNumParams() >= 2 &&
             "ObjC method expected to have implicit parameters (self and _cmd)");
      auto *selfType = llvm::cast<llvm::PointerType>(type->getParamType(0));
      auto *selfStruct = llvm::cast<llvm::StructType>(selfType->getPointerElementType());
      assert(selfStruct->isOpaque() && "ObjC class types expected to be opaque");
      selfStruct->setName(className);
    }
  }
}
