#include "llvm2cpg/CPG/CPGMethod.h"
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>
#include <memory>
#include <set>
#include <string>

using namespace llvm2cpg;

/// Helper methods for debugging

std::string llvm2cpg::valueToString(llvm::Value *value) {
  if (!value) {
    return std::string("<root>");
  }
  std::string str;
  llvm::raw_string_ostream stream(str);
  value->print(stream);
  stream.flush();
  // trim leading whitespaces
  auto firstSymbol = str.find_first_not_of(' ');
  assert(firstSymbol != std::string::npos);
  str.erase(str.begin(), str.begin() + firstSymbol);
  return str;
}

static void dumpTreeMemo(ASTNode *root, int level, std::set<ASTNode *> &visited) {
  llvm::errs().indent(level * 2);
  llvm::errs() << valueToString(root->getValue());
  llvm::errs() << "\n";
  if (visited.count(root) != 0) {
    return;
  }
  visited.insert(root);
  for (auto child : root->getChildren()) {
    dumpTreeMemo(child, level + 1, visited);
  }
}

static void dumpTree(ASTNode *root, int level) {
  std::set<ASTNode *> visited;
  dumpTreeMemo(root, level, visited);
}

#pragma mark - ASTNode

ASTNode::ASTNode(llvm::Value *value) : value(value), children(), parent(nullptr) {}

void ASTNode::addChild(ASTNode *child) {
  assert(child);
  children.push_back(child);
  if (!child->parent) {
    child->parent = this;
  }
}

void ASTNode::setParent(ASTNode *newParent) {
  this->parent = newParent;
}

const std::vector<ASTNode *> &ASTNode::getChildren() const {
  return children;
}

llvm::Value *ASTNode::getValue() const {
  return value;
}

ASTNode *ASTNode::getParent() const {
  return parent;
}

#pragma mark - AST

CPGMethod::CPGMethod(llvm::Function &function)
    : function(function), ownedNodes(), memoization(), root(newNode(nullptr)), types(),
      name(function.getName().str()) {
  constructAST();
}

CPGMethod::CPGMethod(CPGMethod &&that) noexcept
    : function(that.function), ownedNodes(std::move(that.ownedNodes)),
      memoization(std::move(that.memoization)), root(that.root), name(std::move(that.name)) {}

void CPGMethod::constructAST() {
  auto &blocks = function.getBasicBlockList();
  for (auto blockIt = blocks.rbegin(); blockIt != blocks.rend(); ++blockIt) {
    auto &instructions = blockIt->getInstList();
    for (auto instIt = instructions.rbegin(); instIt != instructions.rend(); ++instIt) {
      auto subtree = buildTree(&*instIt);
      if (subtree && !subtree->getParent()) {
        root->addChild(subtree);
      }
    }
  }
}

static bool shouldIgnoreValue(llvm::Value *value) {
  if (llvm::isa<llvm::BasicBlock>(value)) {
    return true;
  }
  if (llvm::isa<llvm::BranchInst>(value)) {
    return true;
  }
  return false;
}

ASTNode *CPGMethod::buildTree(llvm::Value *value) {
  assert(value);
  if (shouldIgnoreValue(value)) {
    return nullptr;
  }

  if (memoization.count(value)) {
    return memoization.find(value)->second;
  }

  auto node = newNode(value);
  memoization[node->getValue()] = node;
  types.insert(value->getType());

  if (auto user = llvm::dyn_cast<llvm::User>(value)) {
    for (auto &operand : user->operands()) {
      auto subtree = buildTree(&*operand);
      if (subtree) {
        node->addChild(subtree);
      }
    }
  } else {
    bool shouldSkip = llvm::isa<llvm::Argument>(value);
    if (!shouldSkip) {
      llvm::errs() << "Cannot handle: " << valueToString(value) << "\n";
    }
  }

  return node;
}

ASTNode *CPGMethod::newNode(llvm::Value *value) {
  ownedNodes.push_back(std::make_unique<ASTNode>(value));
  return ownedNodes.back().get();
}

ASTNode *CPGMethod::getRoot() const {
  return root;
}

ASTNode *CPGMethod::getTree(llvm::Value *value) const {
  auto result = memoization.find(value);
  if (result == memoization.end()) {
    return nullptr;
  }
  return result->second;
}

void CPGMethod::dump() const {
  llvm::errs() << "Types: \n";
  for (auto type : types) {
    llvm::errs() << *type << "\n";
  }

  dumpTree(root, 0);
}

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
