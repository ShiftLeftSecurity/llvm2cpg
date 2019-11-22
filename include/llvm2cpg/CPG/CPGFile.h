#pragma once

#include "llvm2cpg/CPG/CPGMethod.h"
#include <string>
#include <vector>

namespace llvm {
class Module;
class Type;
} // namespace llvm

namespace llvm2cpg {

class CPGFile {
public:
  explicit CPGFile(llvm::Module *module);

  CPGFile(CPGFile &&that) noexcept;

  const std::string &getName() const;
  const std::string &getGlobalNamespaceName() const;
  const std::vector<CPGMethod> &getMethods() const;
  const llvm::Module *getModule() const;

  CPGFile &operator=(CPGFile &&) = delete;
  CPGFile(const CPGFile &) = delete;
  CPGFile &operator=(const CPGFile &) = delete;

private:
  std::string name;
  std::string globalNamespaceName;
  llvm::Module *module;
  std::vector<CPGMethod> methods;
};

} // namespace llvm2cpg
