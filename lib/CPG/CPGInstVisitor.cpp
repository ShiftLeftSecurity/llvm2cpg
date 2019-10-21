#include "CPGInstVisitor.h"

using namespace llvm2cpg;

static void setNameIfEmpty(llvm::Value *value, const std::string &name) {
  if (!value->hasName()) {
    value->setName(name);
  }
}

CPGInstVisitor::CPGInstVisitor(std::vector<llvm::Value *> &arguments,
                               std::vector<llvm::Value *> &variables)
    : arguments(arguments), variables(variables) {}

void CPGInstVisitor::visitFunction(llvm::Function &function) {
  for (auto &arg : function.args()) {
    setNameIfEmpty(&arg, "arg");
    arguments.push_back(&arg);
  }
}

void CPGInstVisitor::visitInstruction(llvm::Instruction &instruction) {
  if (!instruction.getType()->isVoidTy()) {
    addTempVariable(&instruction);
  }
}

void CPGInstVisitor::visitAllocaInst(llvm::AllocaInst &value) {
  addLocalVariable(&value);
}

void CPGInstVisitor::visitPHINode(llvm::PHINode &instruction) {
  llvm::report_fatal_error("PHI nodes should be destructed before CPG emission");
}

void CPGInstVisitor::addLocalVariable(llvm::Value *value) {
  setNameIfEmpty(value, "local");
  variables.push_back(value);
}

void CPGInstVisitor::addTempVariable(llvm::Value *value) {
  setNameIfEmpty(value, "tmp");
  variables.push_back(value);
}
