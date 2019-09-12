#pragma once

#include "CPGMethod.h"
#include <string>
#include <vector>

namespace llvm {
class Module;
class Type;
} // namespace llvm

namespace llvm2cpg {

class CPGFile {
public:
  explicit CPGFile(llvm::Module &module);

  CPGFile(CPGFile &&that) noexcept;

  const std::string &getName() const;
  const std::vector<CPGMethod> &getMethods() const;
  const std::set<llvm::Type *> &getTypes() const;

  CPGFile &operator=(CPGFile &&) = delete;
  CPGFile(const CPGFile &) = delete;
  CPGFile &operator=(const CPGFile &) = delete;

private:
  std::string name;
  std::vector<CPGMethod> methods;
  std::set<llvm::Type *> types;
};

} // namespace llvm2cpg
