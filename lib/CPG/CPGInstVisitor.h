#pragma once

#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/InstVisitor.h>
#include <set>
#include <vector>

namespace llvm2cpg {

class CPGInstVisitor : public llvm::InstVisitor<CPGInstVisitor> {
public:
  CPGInstVisitor(std::vector<llvm::Value *> &arguments, std::vector<llvm::Value *> &variables);
  void run(llvm::Function &function);
  void visitInstruction(llvm::Instruction &instruction);
  void visitAllocaInst(llvm::AllocaInst &value);
  void visitPHINode(llvm::PHINode &instruction);
  void visitDbgVariableIntrinsic(llvm::DbgVariableIntrinsic &instruction);

private:
  std::vector<llvm::Value *> &arguments;
  std::vector<llvm::Value *> &variables;
  std::vector<llvm::DILocalVariable *> formalArguments;

  void addLocalVariable(llvm::Value *value);
  void addTempVariable(llvm::Value *value);
};

} // namespace llvm2cpg
