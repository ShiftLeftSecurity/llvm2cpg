#include "CPGInstVisitor.h"

using namespace llvm2cpg;

static void setNameIfEmpty(llvm::Value *value, const std::string &name) {
  if (!value->hasName()) {
    value->setName(name);
  }
}

CPGInstVisitor::CPGInstVisitor(std::vector<llvm::Value *> &arguments,
                               std::vector<llvm::Value *> &variables, std::set<llvm::Type *> &types)
    : arguments(arguments), variables(variables), types(types) {}

void CPGInstVisitor::visitFunction(llvm::Function &function) {
  types.insert(function.getReturnType());
  for (auto &arg : function.args()) {
    setNameIfEmpty(&arg, "arg");
    arguments.push_back(&arg);
  }
}

void CPGInstVisitor::visitInstruction(llvm::Instruction &instruction) {
  if (!instruction.getType()->isVoidTy()) {
    addTempVariable(&instruction);
  }  
  recordTypes(&instruction);
}

void CPGInstVisitor::visitAllocaInst(llvm::AllocaInst &value) {
  addLocalVariable(&value);
  recordTypes(&value);
}

void CPGInstVisitor::visitPHINode(llvm::PHINode &instruction) {
  llvm::report_fatal_error("PHI nodes should be destructed before CPG emission");
}

void CPGInstVisitor::recordTypes(llvm::Instruction *instruction) {
  if (llvm::isa<llvm::BranchInst>(instruction)) {
    return;
  }
  /// TODO: Feels like too much overhead to collect this info here
  /// It might be better to emit it lazily during CPG generation
  for (auto &operand : instruction->operands()) {
    types.insert(operand->getType());
  }
  types.insert(instruction->getType());
}

void CPGInstVisitor::addLocalVariable(llvm::Value *value) {
  setNameIfEmpty(value, "local");
  variables.push_back(value);
}

void CPGInstVisitor::addTempVariable(llvm::Value *value) {
  setNameIfEmpty(value, "tmp");
  variables.push_back(value);
}
