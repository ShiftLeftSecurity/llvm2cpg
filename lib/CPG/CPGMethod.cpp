#include "llvm2cpg/CPG/CPGMethod.h"
#include "CPGInstVisitor.h"

using namespace llvm2cpg;

CPGMethod::CPGMethod(llvm::Function &function) : function(function), arguments(), localVariables() {
  CPGInstVisitor visitor(arguments, localVariables);
  visitor.run(function);
}

CPGMethod::CPGMethod(CPGMethod &&that) noexcept
    : function(that.function), arguments(std::move(that.arguments)),
      localVariables(std::move(that.localVariables)) {}

llvm::Type *CPGMethod::getReturnType() const {
  assert(function.getFunctionType() != nullptr);
  return function.getFunctionType()->getReturnType();
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
