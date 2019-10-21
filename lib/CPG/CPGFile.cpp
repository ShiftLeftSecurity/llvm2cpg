#include "llvm2cpg/CPG/CPGFile.h"
#include "llvm2cpg/CPG/CPGMethod.h"
#include <llvm/IR/Module.h>
#include <utility>

using namespace llvm2cpg;

CPGFile::CPGFile(llvm::Module &module) : name(module.getSourceFileName()) {
  for (auto &function : module) {
    CPGMethod method(function);
    methods.push_back(std::move(method));
  }
}

CPGFile::CPGFile(CPGFile &&that) noexcept
    : name(std::move(that.name)), globalNamespaceName(name + "_global"),
      methods(std::move(that.methods)) {}

const std::string &CPGFile::getName() const {
  return name;
}

const std::string &CPGFile::getGlobalNamespaceName() const {
  return globalNamespaceName;
}

const std::vector<CPGMethod> &CPGFile::getMethods() const {
  return methods;
}
