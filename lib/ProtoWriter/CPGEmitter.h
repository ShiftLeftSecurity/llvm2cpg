#pragma once

#include <llvm/IR/InstVisitor.h>
#include <unordered_map>
#include <unordered_set>

namespace llvm2cpg {

class CPGMethod;
class CPGFile;
class CPGProtoBuilder;
class CPGProtoNode;
class CPGTypeEmitter;
class CPGLogger;

class CPGEmitter : public llvm::InstVisitor<CPGEmitter, CPGProtoNode *> {
  friend llvm::InstVisitor<CPGEmitter, CPGProtoNode *>;

public:
  CPGEmitter(CPGLogger &logger, CPGProtoBuilder &builder, CPGTypeEmitter &typeEmitter,
             const CPGFile &file);
  void emitMethod(const CPGMethod &method);

private:
  CPGLogger &logger;
  CPGProtoBuilder &builder;
  CPGTypeEmitter &typeEmitter;
  const CPGFile &file;

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
  CPGProtoNode *visitInsertValueInst(llvm::InsertValueInst &instruction);
  CPGProtoNode *visitExtractValueInst(llvm::ExtractValueInst &instruction);
  CPGProtoNode *visitUnaryOperator(llvm::UnaryOperator &instruction);
  CPGProtoNode *visitCallInst(llvm::CallInst &instruction);
  CPGProtoNode *visitPHINode(llvm::PHINode &instruction);
  CPGProtoNode *visitBranchInst(llvm::BranchInst &instruction);
  CPGProtoNode *visitIndirectBrInst(llvm::IndirectBrInst &instruction);
  CPGProtoNode *visitSwitchInst(llvm::SwitchInst &instruction);
  CPGProtoNode *visitUnreachableInst(llvm::UnreachableInst &instruction);
  CPGProtoNode *visitAtomicRMWInst(llvm::AtomicRMWInst &instruction);
  CPGProtoNode *visitAtomicCmpXchgInst(llvm::AtomicCmpXchgInst &instruction);
  CPGProtoNode *visitFenceInst(llvm::FenceInst &instruction);

  // No support for llvm9 scalable vectors.
  CPGProtoNode *visitExtractElementInst(llvm::ExtractElementInst &instruction);
  CPGProtoNode *visitInsertElementInst(llvm::InsertElementInst &instruction);
  CPGProtoNode *visitShuffleVectorInst(llvm::ShuffleVectorInst &instruction);



  CPGProtoNode *emitMethodNode(const CPGMethod &method);
  CPGProtoNode *emitMethodReturnNode(const CPGMethod &method);
  CPGProtoNode *emitMethodBlock(const CPGMethod &method);

  CPGProtoNode *emitRefOrConstant(llvm::Value *value);
  CPGProtoNode *emitRef(llvm::Value *value);
  CPGProtoNode *emitConstant(llvm::Value *value);
  CPGProtoNode *emitConstant(unsigned int);
  CPGProtoNode *emitConstantExpr(llvm::ConstantExpr *constantExpr);
  CPGProtoNode *emitLocalVariable(const llvm::Value *variable, size_t order);
  CPGProtoNode *emitFunctionArgument(const llvm::Value *argument, size_t order);

  CPGProtoNode *emitAllocaCall(const llvm::Value *value);
  CPGProtoNode *emitAssignCall(const llvm::Value *value, CPGProtoNode *lhs, CPGProtoNode *rhs);
  CPGProtoNode *emitIndirectionCall(const llvm::Value *value, CPGProtoNode *pointerRef);
  CPGProtoNode *emitDereference(llvm::Value *value);
  CPGProtoNode *emitBinaryCall(const llvm::BinaryOperator *binary);
  CPGProtoNode *emitCmpCall(const llvm::CmpInst *comparison);
  CPGProtoNode *emitCast(const llvm::CastInst *instruction);
  CPGProtoNode *emitSelect(llvm::SelectInst *instruction);
  CPGProtoNode *emitGEP(const llvm::GetElementPtrInst *instruction);
  CPGProtoNode *emitInsertValue(llvm::InsertValueInst *instruction);
  CPGProtoNode *emitExtractValue(llvm::ExtractValueInst *instruction);

  CPGProtoNode *emitGEPAccess(const llvm::Type *type, llvm::Value *index, bool memberAccess);
  CPGProtoNode *emitInsertAccess(const llvm::Type *type, unsigned int idx, bool memberAccess);
  CPGProtoNode *emitExtract(const llvm::Type *type, unsigned int idx, bool memberAccess);
  CPGProtoNode *emitUnaryOperator(const llvm::UnaryOperator *instruction);
  CPGProtoNode *emitFunctionCall(const llvm::CallInst *instruction);
  CPGProtoNode *emitNoop();
  CPGProtoNode *emitUnhandled();
  CPGProtoNode *emitAtomicRMW(llvm::AtomicRMWInst *instruction);
  CPGProtoNode *emitAtomicCmpXchg(llvm::AtomicCmpXchgInst *instruction);
  CPGProtoNode *emitFence(const llvm::FenceInst *instruction);

  CPGProtoNode *emitExtractElement(llvm::ExtractElementInst *instruction);  
  CPGProtoNode *emitInsertElement(llvm::InsertElementInst *instruction);  
  CPGProtoNode *emitShuffleVector(llvm::ShuffleVectorInst *instruction);  


  // Returns true if the value is a local variable or an argument, false otherwise
  bool isLocal(const llvm::Value *value) const;
  // Returns true if the value is a global variable, false otherwise
  bool isGlobal(const llvm::Value *value) const;
  bool isConstExpr(const llvm::Value *value) const;

  // Returns a reference to the emitted local variable or argument
  const CPGProtoNode *getLocal(const llvm::Value *value) const;

  // Sets the right CFG/AST connections
  void resolveConnections(CPGProtoNode *parent, std::vector<CPGProtoNode *> children);
  void resolveCFGConnections(CPGProtoNode *parent, std::vector<CPGProtoNode *> children);
  void resolveASTConnections(CPGProtoNode *parent, std::vector<CPGProtoNode *> children);

  std::string getTypeName(const llvm::Type *type);
};

} // namespace llvm2cpg
