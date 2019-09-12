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

/// For debugging/tests only
std::string valueToString(llvm::Value *value);

class ASTNode {
public:
  explicit ASTNode(llvm::Value *value);

  void addChild(ASTNode *child);
  void setParent(ASTNode *newParent);

  const std::vector<ASTNode *> &getChildren() const;
  ASTNode *getParent() const;
  llvm::Value *getValue() const;

private:
  llvm::Value *value;
  std::vector<ASTNode *> children;
  ASTNode *parent;
};

class CPGMethod {
public:
  explicit CPGMethod(llvm::Function &function);

  CPGMethod(CPGMethod &&that) noexcept;

  CPGMethod &operator=(CPGMethod &&) = delete;
  CPGMethod(const CPGMethod &) = delete;
  CPGMethod &operator=(const CPGMethod &) = delete;

  ASTNode *getRoot() const;
  ASTNode *getTree(llvm::Value *value) const;
  const std::set<llvm::Type *> &getTypes() const;
  const std::string &getName() const;
  const std::string &getSignature() const;
  bool isExternal() const;

  void dump() const;

private:
  void constructAST();
  ASTNode *buildTree(llvm::Value *value);
  ASTNode *newNode(llvm::Value *value);

  llvm::Function &function;
  std::vector<std::unique_ptr<ASTNode>> ownedNodes;
  std::map<llvm::Value *, ASTNode *> memoization;
  ASTNode *root;
  std::set<llvm::Type *> types;
  std::string name;
};

} // namespace llvm2cpg
