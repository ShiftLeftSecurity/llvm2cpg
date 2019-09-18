#include "llvm2cpg/CPG/CPGMethod.h"
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>
#include <memory>
#include <set>
#include <string>

using namespace llvm2cpg;

CPGMethod::CPGMethod(llvm::Function &function)
    : function(function), types(), name(function.getName().str()) {}

CPGMethod::CPGMethod(CPGMethod &&that) noexcept
    : function(that.function), name(std::move(that.name)) {}

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

const llvm::Function &CPGMethod::getFunction() const {
  return function;
}
