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

  const std::vector<llvm::Value *> &getArguments() const;
  const std::vector<llvm::Value *> &getLocalVariables() const;

private:
  llvm::Function &function;
  std::set<llvm::Type *> types;
  std::string name;

  std::vector<llvm::Value *> arguments;
  std::vector<llvm::Value *> localVariables;
};

} // namespace llvm2cpg
