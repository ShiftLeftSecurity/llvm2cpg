#include "llvm2cpg/CPG/CPGFile.h"
#include "llvm2cpg/CPG/CPGMethod.h"
#include <llvm/IR/Module.h>
#include <utility>

using namespace llvm2cpg;

CPGFile::CPGFile(llvm::Module &module) : name(module.getSourceFileName()) {
  for (auto &function : module) {
    CPGMethod method(function);
    std::copy(std::begin(method.getTypes()),
              std::end(method.getTypes()),
              std::inserter(types, types.begin()));
    methods.push_back(std::move(method));
  }
}

CPGFile::CPGFile(CPGFile &&that) noexcept
    : name(std::move(that.name)), methods(std::move(that.methods)), types(std::move(that.types)) {}

const std::string &CPGFile::getName() const {
  return name;
}

const std::vector<CPGMethod> &CPGFile::getMethods() const {
  return methods;
}

const std::set<llvm::Type *> &CPGFile::getTypes() const {
  return types;
}
