#pragma once

#include "CPGProtoBuilder.h"
#include "CPGProtoNode.h"
#include <llvm/IR/InstVisitor.h>

namespace llvm2cpg {

class CPGMethod;
class CPGProtoNode;

class CPGEmitter : public llvm::InstVisitor<CPGEmitter, CPGProtoNode *> {
  friend llvm::InstVisitor<CPGEmitter, CPGProtoNode *>;

public:
  explicit CPGEmitter(CPGProtoBuilder &builder);
  void emitMethod(const CPGMethod &method);

private:
  CPGProtoBuilder &builder;

  std::unordered_map<const llvm::Value *, CPGProtoNode *> locals;
  std::unordered_set<const llvm::Value *> globals;

  CPGProtoNode *visitInstruction(llvm::Instruction &instruction);
  CPGProtoNode *visitReturnInst(llvm::ReturnInst &instruction);
  CPGProtoNode *visitAllocaInst(llvm::AllocaInst &instruction);
  CPGProtoNode *visitStoreInst(llvm::StoreInst &instruction);
  CPGProtoNode *visitLoadInst(llvm::LoadInst &instruction);
  CPGProtoNode *visitBinaryOperator(llvm::BinaryOperator &instruction);
  CPGProtoNode *visitCmpInst(llvm::CmpInst &instruction);
  CPGProtoNode *visitCastInst(llvm::CastInst &instruction);
  CPGProtoNode *visitSelectInst(llvm::SelectInst &instruction);
  CPGProtoNode *visitGetElementPtrInst(llvm::GetElementPtrInst &instruction);

  CPGProtoNode *emitMethodNode(const CPGMethod &method);
  CPGProtoNode *emitMethodReturnNode(const CPGMethod &method);
  CPGProtoNode *emitMethodBlock(const CPGMethod &method);

  CPGProtoNode *emitRefOrConstant(const llvm::Value *value);
  CPGProtoNode *emitRef(const llvm::Value *value);
  CPGProtoNode *emitConstant(const llvm::Value *value);
  CPGProtoNode *emitLocalVariable(const llvm::Value *variable, size_t order);
  CPGProtoNode *emitFunctionArgument(const llvm::Value *argument, size_t order);

  CPGProtoNode *emitAllocaCall(const llvm::Value *value);
  CPGProtoNode *emitAssignCall(const llvm::Value *value, CPGProtoNode *lhs, CPGProtoNode *rhs);
  CPGProtoNode *emitIndirectionCall(const llvm::Value *value, CPGProtoNode *pointerRef);
  CPGProtoNode *emitDereference(const llvm::Value *value);
  CPGProtoNode *emitBinaryCall(const llvm::BinaryOperator *binary);
  CPGProtoNode *emitCmpCall(const llvm::CmpInst *comparison);
  CPGProtoNode *emitCast(const llvm::CastInst *instruction);
  CPGProtoNode *emitSelect(const llvm::SelectInst *instruction);
  CPGProtoNode *emitGEP(const llvm::GetElementPtrInst *instruction);
  CPGProtoNode *emitGEPAccess(const llvm::Type *type, llvm::Value *index, bool memberAccess);

  // Returns true if the value is a local variable or an argument, false otherwise
  bool isLocal(const llvm::Value *value) const;
  // Returns true if the value is a global variable, false otherwise
  bool isGlobal(const llvm::Value *value) const;

  // Returns a reference to the emitted local variable or argument
  const CPGProtoNode *getLocal(const llvm::Value *value) const;

  // Sets the right CFG/AST connections
  void resolveConnections(CPGProtoNode *parent, std::vector<CPGProtoNode *> children);
};

} // namespace llvm2cpg
