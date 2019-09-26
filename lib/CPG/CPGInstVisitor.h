#pragma once

#include <llvm/IR/InstVisitor.h>
#include <set>
#include <vector>

namespace llvm2cpg {

class CPGInstVisitor : public llvm::InstVisitor<CPGInstVisitor> {
public:
  CPGInstVisitor(std::vector<llvm::Value *> &arguments, std::vector<llvm::Value *> &variables,
                 std::set<llvm::Type *> &types);

  void visitFunction(llvm::Function &function);

  void visitInstruction(llvm::Instruction &instruction);
  void visitAllocaInst(llvm::AllocaInst &value);
  void visitLoadInst(llvm::LoadInst &value);
  void visitBinaryOperator(llvm::BinaryOperator &binaryOperator);
  void visitCmpInst(llvm::CmpInst &comparison);

  void visitCastInst(llvm::CastInst &instruction);

private:
  std::vector<llvm::Value *> &arguments;
  std::vector<llvm::Value *> &variables;
  std::set<llvm::Type *> &types;

  void addLocalVariable(llvm::Value *value);
  void addTempVariable(llvm::Value *value);
  void recordTypes(llvm::Instruction *instruction);
};

} // namespace llvm2cpg
