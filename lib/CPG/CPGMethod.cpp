#include "llvm2cpg/CPG/CPGMethod.h"
#include "CPGInstVisitor.h"

using namespace llvm2cpg;

CPGMethod::CPGMethod(llvm::Function &function)
    : function(function), types(), name(function.getName().str()), arguments(), localVariables() {
  CPGInstVisitor visitor(arguments, localVariables, types);
  visitor.visit(function);
}

CPGMethod::CPGMethod(CPGMethod &&that) noexcept
    : function(that.function), types(std::move(that.types)), name(std::move(that.name)),
      arguments(std::move(that.arguments)), localVariables(std::move(that.localVariables)) {}

const std::set<llvm::Type *> &CPGMethod::getTypes() const {
  return types;
}

llvm::Type *CPGMethod::getReturnType() const {
  assert(function.getFunctionType() != nullptr);
  return function.getFunctionType()->getReturnType();
}

const std::string &CPGMethod::getName() const {
  return name;
}

const std::string &CPGMethod::getSignature() const {
  return getName();
}

bool CPGMethod::isExternal() const {
  return function.isDeclaration();
}

llvm::Function &CPGMethod::getFunction() const {
  return function;
}

const std::vector<llvm::Value *> &CPGMethod::getArguments() const {
  return arguments;
}

const std::vector<llvm::Value *> &CPGMethod::getLocalVariables() const {
  return localVariables;
}

llvm::Instruction *CPGMethod::getEntryInstruction() const {
  assert(!function.isDeclaration() && "Cannot get entry instruction from function declaration");
  return &function.getEntryBlock().front();
}

std::vector<llvm::Instruction *> CPGMethod::getReturnInstructions() const {
  assert(!function.isDeclaration() && "Cannot get return instruction from function declaration");
  std::vector<llvm::Instruction *> instructions;
  for (llvm::BasicBlock &block : function.getBasicBlockList()) {
    llvm::Instruction *terminator = block.getTerminator();
    assert(terminator && "Malformed basic block");
    if (llvm::isa<llvm::ReturnInst>(terminator)) {
      instructions.push_back(terminator);
    }
  }
  return instructions;
}
