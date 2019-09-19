#pragma once

#include <llvm/IR/InstVisitor.h>
#include <set>
#include <vector>

namespace llvm2cpg {

class CPGInstVisitor : public llvm::InstVisitor<CPGInstVisitor> {
public:
  explicit CPGInstVisitor(std::vector<llvm::Value *> &arguments,
                          std::vector<llvm::Value *> &variables, std::set<llvm::Type *> &types);

  void visitFunction(llvm::Function &function);

  void visitInstruction(llvm::Instruction &instruction);
  void visitAllocaInst(llvm::AllocaInst &value);
  void visitLoadInst(llvm::LoadInst &value);

private:
  std::vector<llvm::Value *> &arguments;
  std::vector<llvm::Value *> &variables;
  std::set<llvm::Type *> &types;

  void addLocalVariable(llvm::Value *value);
  void addTempVariable(llvm::Value *value);
  void setNameIfEmpty(llvm::Value *value, const std::string &name);
  void recordTypes(llvm::Instruction *instruction);
};

} // namespace llvm2cpg
