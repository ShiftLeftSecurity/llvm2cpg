#pragma once

#include <map>
#include <memory>
#include <set>
#include <vector>

namespace llvm {
class Function;
class Value;
class Type;
} // namespace llvm

namespace llvm2cpg {

class CPGMethod {
public:
  explicit CPGMethod(llvm::Function &function);

  CPGMethod(CPGMethod &&that) noexcept;

  CPGMethod &operator=(CPGMethod &&) = delete;
  CPGMethod(const CPGMethod &) = delete;
  CPGMethod &operator=(const CPGMethod &) = delete;

  const std::set<llvm::Type *> &getTypes() const;
  llvm::Type *getReturnType() const;
  const std::string &getName() const;
  const std::string &getSignature() const;
  bool isExternal() const;
  const llvm::Function &getFunction() const;

private:
  llvm::Function &function;
  std::set<llvm::Type *> types;
  std::string name;
};

} // namespace llvm2cpg
