#include "llvm2cpg/Transforms/Transforms.h"
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/Transforms/Utils/Local.h>

using namespace llvm2cpg;

void Transforms::transformBitcode(llvm::Module &bitcode) {
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
