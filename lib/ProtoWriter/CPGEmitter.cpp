#include "CPGEmitter.h"
#include "CPGProtoBuilder.h"
#include "CPGProtoNode.h"
#include "CPGTypeEmitter.h"
#include "llvm2cpg/CPG/CPGFile.h"
#include "llvm2cpg/CPG/CPGMethod.h"

using namespace llvm2cpg;

static std::string valueToString(const llvm::Value *value) {
  std::string name;
  llvm::raw_string_ostream stream(name);
  value->printAsOperand(stream);
  return stream.str();
}

CPGEmitter::CPGEmitter(CPGProtoBuilder &builder, CPGTypeEmitter &typeEmitter, const CPGFile &file)
    : builder(builder), typeEmitter(typeEmitter), file(file) {}

void CPGEmitter::emitMethod(const CPGMethod &method) {
  CPGProtoNode *methodNode = emitMethodNode(method);
  CPGProtoNode *methodReturnNode = emitMethodReturnNode(method);

  CPGProtoNode *methodBlock = emitMethodBlock(method);

  builder.connectAST(methodNode, methodBlock);
  builder.connectAST(methodNode, methodReturnNode);

  for (size_t argIndex = 0; argIndex < method.getArguments().size(); argIndex++) {
    llvm::Value *argument = method.getArguments()[argIndex];
    CPGProtoNode *parameterInNode = emitFunctionArgument(argument, argIndex + 1);
    builder.connectAST(methodNode, parameterInNode);
    locals.insert(std::make_pair(argument, parameterInNode));
  }

  /// Skipping method declaration (empty methods)
  if (method.getFunction().isDeclaration()) {
    return;
  }

  llvm::errs() << "Emitting " << method.getName() << "\n";

  llvm::Module *module = method.getFunction().getParent();
  for (llvm::GlobalVariable &global : module->getGlobalList()) {
    globals.insert(&global);
  }

  for (size_t localIndex = 0; localIndex < method.getLocalVariables().size(); localIndex++) {
    llvm::Value *variable = method.getLocalVariables()[localIndex];
    CPGProtoNode *local = emitLocalVariable(variable, localIndex);
    builder.connectAST(methodBlock, local);
    locals.insert(std::make_pair(variable, local));
  }

  std::unordered_map<const llvm::Value *, CPGProtoNode *> topLevelNodes;
  std::vector<const llvm::BranchInst *> unresolvedBranches;
  std::vector<const llvm::SwitchInst *> unresolvedSwitches;

  size_t topLevelOrder = 1;
  for (llvm::BasicBlock &basicBlock : method.getFunction()) {
    std::vector<CPGProtoNode *> nodes;
    for (llvm::Instruction &instruction : basicBlock) {
      // We cannot make CFG connections for the terminators yet
      // So we collect them for later use
      if (auto branch = llvm::dyn_cast<llvm::BranchInst>(&instruction)) {
        unresolvedBranches.push_back(branch);
      }
      if (auto switchInst = llvm::dyn_cast<llvm::SwitchInst>(&instruction)) {
        unresolvedSwitches.push_back(switchInst);
      }
      // TODO: Switch to llvm::Optional?
      // visit returns a CPG node for the instruction. If we ignore or do not support an
      // instruction, then we get nullptr
      CPGProtoNode *node = visit(instruction);
      if (!node) {
        continue;
      }
      node->setOrderAndIndex(topLevelOrder++);
      nodes.push_back(node);
      topLevelNodes.insert(std::make_pair(&instruction, node));
    }

    if (nodes.empty()) {
      continue;
    }

    // Connect AST
    for (CPGProtoNode *node : nodes) {
      builder.connectAST(methodBlock, node);
    }

    // Connect CFG across the Basic Block instructions
    for (size_t i = 0; i < nodes.size() - 1; i++) {
      CPGProtoNode *current = nodes[i];
      CPGProtoNode *next = nodes[i + 1];
      builder.connectCFG(current->getID(), next->getEntry());
    }
  }

  // Connect CFG: method -> method body -> method return
  llvm::Instruction *entryInstruction = method.getEntryInstruction();
  assert(topLevelNodes.count(entryInstruction) != 0);
  CPGProtoNode *head = topLevelNodes.at(entryInstruction);
  builder.connectCFG(methodNode->getID(), head->getEntry());

  for (llvm::Instruction *returnInstruction : method.getReturnInstructions()) {
    assert(llvm::isa<llvm::ReturnInst>(returnInstruction));
    assert(topLevelNodes.count(returnInstruction) != 0);
    CPGProtoNode *retNode = topLevelNodes.at(returnInstruction);
    builder.connectCFG(retNode->getID(), methodReturnNode->getID());
  }

  // Connect CFG for unresolved branches
  for (const llvm::BranchInst *branch : unresolvedBranches) {
    const llvm::Value *sourceInstruction = nullptr;
    // The branch condition can be a const expression, which is not an instruction
    // In this case we consider the previous instruction to be the source of CFG edges
    if (branch->isConditional() && llvm::isa<llvm::Instruction>(branch->getCondition())) {
      sourceInstruction = branch->getCondition();
    } else if (branch->getPrevNonDebugInstruction()) {
      sourceInstruction = branch->getPrevNonDebugInstruction();
    } else {
      sourceInstruction = branch;
    }
    assert(topLevelNodes.count(sourceInstruction));
    CPGProtoNode *source = topLevelNodes.at(sourceInstruction);
    for (size_t i = 0; i < branch->getNumSuccessors(); i++) {
      llvm::BasicBlock *successor = branch->getSuccessor(i);
      assert(topLevelNodes.count(&successor->front()) && "cannot connect successor");
      CPGProtoNode *destination = topLevelNodes.at(&successor->front());
      builder.connectCFG(source->getID(), destination->getEntry());
    }
  }

  // Connect CFG for unresolved switches
  for (const llvm::SwitchInst *switchInst : unresolvedSwitches) {
    const llvm::Value *sourceInstruction = nullptr;
    if (llvm::isa<llvm::Instruction>(switchInst->getCondition())) {
      sourceInstruction = switchInst->getCondition();
    } else if (switchInst->getPrevNonDebugInstruction()) {
      sourceInstruction = switchInst->getPrevNonDebugInstruction();
    } else {
      sourceInstruction = switchInst;
    }
    assert(topLevelNodes.count(sourceInstruction));
    CPGProtoNode *source = topLevelNodes.at(sourceInstruction);
    for (size_t i = 0; i < switchInst->getNumSuccessors(); i++) {
      const llvm::BasicBlock *successor = switchInst->getSuccessor(i);
      assert(topLevelNodes.count(&successor->front()));
      CPGProtoNode *destination = topLevelNodes.at(&successor->front());
      builder.connectCFG(source->getID(), destination->getEntry());
    }
  }
}

CPGProtoNode *CPGEmitter::visitInstruction(llvm::Instruction &instruction) {
  llvm::errs() << "Cannot handle: " << instruction << ". Skipping.\n";
  CPGProtoNode *unhandled = builder.unknownNode();
  resolveConnections(unhandled, {});

  return unhandled;
}

CPGProtoNode *CPGEmitter::visitAllocaInst(llvm::AllocaInst &instruction) {
  CPGProtoNode *localRef = emitRef(&instruction);
  CPGProtoNode *allocaCall = emitAllocaCall(&instruction);
  CPGProtoNode *assignCall = emitAssignCall(&instruction, localRef, allocaCall);
  return assignCall;
}

CPGProtoNode *CPGEmitter::visitStoreInst(llvm::StoreInst &instruction) {
  llvm::Value *value = instruction.getValueOperand();
  llvm::Value *pointer = instruction.getPointerOperand();

  CPGProtoNode *valueRef = emitRefOrConstant(value);
  CPGProtoNode *addressRef = emitRef(pointer);
  CPGProtoNode *indirectionCall = emitIndirectionCall(value, addressRef);
  CPGProtoNode *assignCall = emitAssignCall(pointer, indirectionCall, valueRef);
  return assignCall;
}

CPGProtoNode *CPGEmitter::visitLoadInst(llvm::LoadInst &instruction) {
  llvm::Value *pointer = instruction.getPointerOperand();
  CPGProtoNode *tempRef = emitRef(&instruction);
  CPGProtoNode *dereferenceCall = emitDereference(pointer);
  CPGProtoNode *assignCall = emitAssignCall(pointer, tempRef, dereferenceCall);
  return assignCall;
}

CPGProtoNode *CPGEmitter::visitBinaryOperator(llvm::BinaryOperator &instruction) {
  CPGProtoNode *binCall = emitBinaryCall(&instruction);
  CPGProtoNode *binRef = emitRef(&instruction);
  CPGProtoNode *assignCall = emitAssignCall(&instruction, binRef, binCall);
  return assignCall;
}

CPGProtoNode *CPGEmitter::visitCmpInst(llvm::CmpInst &instruction) {
  CPGProtoNode *cmpCall = emitCmpCall(&instruction);
  CPGProtoNode *cmpRef = emitRef(&instruction);
  CPGProtoNode *assignCall = emitAssignCall(&instruction, cmpRef, cmpCall);
  return assignCall;
}

CPGProtoNode *CPGEmitter::visitCastInst(llvm::CastInst &instruction) {
  CPGProtoNode *ref = emitRef(&instruction);
  CPGProtoNode *castCall = emitCast(&instruction);
  CPGProtoNode *assignCall = emitAssignCall(&instruction, ref, castCall);
  return assignCall;
}

CPGProtoNode *CPGEmitter::visitSelectInst(llvm::SelectInst &instruction) {
  CPGProtoNode *ref = emitRef(&instruction);
  CPGProtoNode *selectCall = emitSelect(&instruction);
  CPGProtoNode *assignCall = emitAssignCall(&instruction, ref, selectCall);
  return assignCall;
}

CPGProtoNode *CPGEmitter::visitGetElementPtrInst(llvm::GetElementPtrInst &instruction) {
  CPGProtoNode *ref = emitRef(&instruction);
  CPGProtoNode *call = emitGEP(&instruction);
  CPGProtoNode *assignCall = emitAssignCall(&instruction, ref, call);
  return assignCall;
}

CPGProtoNode *CPGEmitter::visitInsertValueInst(llvm::InsertValueInst &instruction) {
  CPGProtoNode *ref = emitRef(&instruction);
  CPGProtoNode *call = emitInsertValue(&instruction);
  CPGProtoNode *assignCall = emitAssignCall(&instruction, ref, call);
  return assignCall;
}

CPGProtoNode *CPGEmitter::visitExtractValueInst(llvm::ExtractValueInst &instruction) {
  CPGProtoNode *ref = emitRef(&instruction);
  CPGProtoNode *call = emitExtractValue(&instruction);
  CPGProtoNode *assignCall = emitAssignCall(&instruction, ref, call);
  return assignCall;
}

CPGProtoNode *CPGEmitter::visitUnaryOperator(llvm::UnaryOperator &instruction) {
  CPGProtoNode *ref = emitRef(&instruction);
  CPGProtoNode *call = emitUnaryOperator(&instruction);
  CPGProtoNode *assignCall = emitAssignCall(&instruction, ref, call);
  return assignCall;
}

CPGProtoNode *CPGEmitter::visitCallInst(llvm::CallInst &instruction) {
  if (instruction.getFunctionType()->getReturnType()->isVoidTy()) {
    return emitFunctionCall(&instruction);
  }
  CPGProtoNode *ref = emitRef(&instruction);
  CPGProtoNode *call = emitFunctionCall(&instruction);
  CPGProtoNode *assignCall = emitAssignCall(&instruction, ref, call);
  return assignCall;
}

CPGProtoNode *CPGEmitter::visitAtomicRMWInst(llvm::AtomicRMWInst &instruction) {
  CPGProtoNode *ref = emitRef(&instruction);
  CPGProtoNode *call = emitAtomicRMW(&instruction);
  CPGProtoNode *assignCall = emitAssignCall(&instruction, ref, call);
  return assignCall;
}

CPGProtoNode *CPGEmitter::emitAtomicRMW(llvm::AtomicRMWInst *instruction) {
  CPGProtoNode *acall = builder.functionCallNode();
  std::string name(instruction->getOpcodeName());
  name = name + std::string(instruction->getOperationName(instruction->getOperation()));

  (*acall) //
      .setName(name)
      .setCode(name)
      .setTypeFullName(getTypeName(instruction->getType()))
      .setMethodInstFullName(name)
      .setMethodFullName(name)
      .setSignature("xxx")
      .setDispatchType("STATIC_DISPATCH");

  CPGProtoNode *ptr = emitRefOrConstant(instruction->getPointerOperand());
  CPGProtoNode *val = emitRefOrConstant(instruction->getValOperand());

  resolveConnections(acall, { ptr, val });
  return acall;
}

CPGProtoNode *CPGEmitter::visitAtomicCmpXchgInst(llvm::AtomicCmpXchgInst &instruction) {
  CPGProtoNode *ref = emitRef(&instruction);
  CPGProtoNode *call = emitAtomicCmpXchg(&instruction);
  CPGProtoNode *assignCall = emitAssignCall(&instruction, ref, call);
  return assignCall;
}

CPGProtoNode *CPGEmitter::emitAtomicCmpXchg(llvm::AtomicCmpXchgInst *instruction) {
  std::string name(instruction->getOpcodeName());
  CPGProtoNode *acall = builder.functionCallNode();

  (*acall) //
      .setName(name)
      .setCode(name)
      .setTypeFullName(getTypeName(instruction->getType()))
      .setMethodInstFullName(name)
      .setName(name)
      .setSignature("xxx")
      .setDispatchType("STATIC_DISPATCH");

  CPGProtoNode *ptr = emitRefOrConstant(instruction->getPointerOperand());
  CPGProtoNode *val = emitRefOrConstant(instruction->getCompareOperand());
  resolveConnections(acall, { ptr, val });
  return acall;
}

CPGProtoNode *CPGEmitter::visitExtractElementInst(llvm::ExtractElementInst &instruction) {
  CPGProtoNode *ref = emitRef(&instruction);
  CPGProtoNode *call = emitExtractElement(&instruction);
  CPGProtoNode *assignCall = emitAssignCall(&instruction, ref, call);
  return assignCall;
}

CPGProtoNode *CPGEmitter::emitExtractElement(llvm::ExtractElementInst *instruction) {
  CPGProtoNode *acall = builder.functionCallNode();
  (*acall) //
      .setName("extractelement")
      .setCode("extractelement")
      .setTypeFullName(getTypeName(instruction->getType()))
      .setMethodInstFullName("extractelement")
      .setSignature("xxx")
      .setDispatchType("STATIC_DISPATCH");
  CPGProtoNode *vec = emitRefOrConstant(instruction->getVectorOperand());
  CPGProtoNode *idx = emitRefOrConstant(instruction->getIndexOperand());
  resolveConnections(acall, { vec, idx });
  return acall;
}

CPGProtoNode *CPGEmitter::visitInsertElementInst(llvm::InsertElementInst &instruction) {
  CPGProtoNode *ref = emitRef(&instruction);
  CPGProtoNode *call = emitInsertElement(&instruction);
  CPGProtoNode *assignCall = emitAssignCall(&instruction, ref, call);
  return assignCall;
}

CPGProtoNode *CPGEmitter::emitInsertElement(llvm::InsertElementInst *instruction) {
  CPGProtoNode *acall = builder.functionCallNode();
  (*acall) //
      .setName("insertelement")
      .setCode("insertelement")
      .setTypeFullName(getTypeName(instruction->getType()))
      .setMethodInstFullName("insertelement")
      .setSignature("xxx")
      .setDispatchType("STATIC_DISPATCH");
  // operand ordering cf llvm/IR/Instructions.h
  // llvm ran out of budget for named accessor functions, hence this.
  CPGProtoNode *vec = emitRefOrConstant(instruction->getOperand(0));
  CPGProtoNode *val = emitRefOrConstant(instruction->getOperand(1));
  CPGProtoNode *idx = emitRefOrConstant(instruction->getOperand(2));
  resolveConnections(acall, { vec, val, idx });
  return acall;
}

CPGProtoNode *CPGEmitter::visitShuffleVectorInst(llvm::ShuffleVectorInst &instruction) {
  CPGProtoNode *ref = emitRef(&instruction);
  CPGProtoNode *call = emitShuffleVector(&instruction);
  CPGProtoNode *assignCall = emitAssignCall(&instruction, ref, call);
  return assignCall;
}

CPGProtoNode *CPGEmitter::emitShuffleVector(llvm::ShuffleVectorInst *instruction) {
  CPGProtoNode *acall = builder.functionCallNode();
  (*acall) //
      .setName("shufflevector")
      .setCode("shufflevector")
      .setTypeFullName(getTypeName(instruction->getType()))
      .setMethodInstFullName("shufflevector")
      .setSignature("xxx")
      .setDispatchType("STATIC_DISPATCH");
  // operand ordering cf llvm/IR/Instructions.h
  // llvm ran out of budget for named accessor functions, hence this.
  CPGProtoNode *vec1 = emitRefOrConstant(instruction->getOperand(0));
  CPGProtoNode *vec2 = emitRefOrConstant(instruction->getOperand(1));
  CPGProtoNode *msk = emitRefOrConstant(instruction->getOperand(2));
  resolveConnections(acall, { vec1, vec2, msk });
  return acall;
}

CPGProtoNode *CPGEmitter::visitPHINode(llvm::PHINode &instruction) {
  llvm::report_fatal_error("PHI nodes should be destructed before CPG emission");
  return nullptr;
}

CPGProtoNode *CPGEmitter::visitBranchInst(llvm::BranchInst &instruction) {
  /// Emit noop CPG node to reflect the 'empty' basic block:
  /// a basic block with only branch instruction
  if (!instruction.getPrevNonDebugInstruction()) {
    return emitNoop();
  }
  return nullptr;
}

CPGProtoNode *CPGEmitter::visitSwitchInst(llvm::SwitchInst &instruction) {
  /// Emit noop CPG node to reflect the 'empty' basic block:
  /// a basic block with only branch instruction
  if (!instruction.getPrevNonDebugInstruction()) {
    return emitNoop();
  }
  return nullptr;
}

CPGProtoNode *CPGEmitter::visitUnreachableInst(llvm::UnreachableInst &instruction) {
  CPGProtoNode *node = emitUnhandled();
  node->setCode("unreachable");
  resolveConnections(node, {});
  return node;
}

CPGProtoNode *CPGEmitter::visitFenceInst(llvm::FenceInst &instruction) {
  return emitFence(&instruction);
}

CPGProtoNode *CPGEmitter::emitFence(const llvm::FenceInst *instruction) {
  CPGProtoNode *callNode = builder.functionCallNode();
  (*callNode) //
      .setName("fence")
      .setCode("fence")
      .setTypeFullName(getTypeName(instruction->getType()))
      .setMethodInstFullName("fence")
      .setSignature("xxx")
      .setDispatchType("STATIC_DISPATCH");
  resolveConnections(callNode, {});
  return callNode;
}

CPGProtoNode *CPGEmitter::visitReturnInst(llvm::ReturnInst &instruction) {
  CPGProtoNode *returnNode = builder.returnNode();
  returnNode->setCode("return");

  std::vector<CPGProtoNode *> children;
  if (llvm::Value *retValue = instruction.getReturnValue()) {
    CPGProtoNode *retValueNode = emitRefOrConstant(retValue);
    children.push_back(retValueNode);
  }
  resolveConnections(returnNode, children);
  return returnNode;
}

CPGProtoNode *CPGEmitter::emitMethodNode(const CPGMethod &method) {
  CPGProtoNode *methodNode = builder.methodNode();
  (*methodNode) //
      .setName(method.getName())
      .setFullName(method.getName())
      .setASTParentType("NAMESPACE_BLOCK")
      .setASTParentFullName(file.getGlobalNamespaceName())
      .setIsExternal(method.isExternal())
      .setSignature(method.getSignature())
      .setOrder(0);
  return methodNode;
}

CPGProtoNode *CPGEmitter::emitMethodReturnNode(const CPGMethod &method) {
  std::string methodReturnType = getTypeName(method.getReturnType());
  CPGProtoNode *methodReturnNode = builder.methodReturnNode();
  (*methodReturnNode) //
      .setOrder(0)
      .setTypeFullName(methodReturnType)
      .setCode(methodReturnType)
      .setEvaluationStrategy("EVAL");
  return methodReturnNode;
}

CPGProtoNode *CPGEmitter::emitMethodBlock(const CPGMethod &method) {
  CPGProtoNode *blockNode = builder.methodBlockNode();
  std::string methodReturnType = getTypeName(method.getReturnType());
  (*blockNode) //
      .setOrder(0)
      .setTypeFullName(methodReturnType)
      .setCode("x + 42")
      .setArgumentIndex(0);
  return blockNode;
}

CPGProtoNode *CPGEmitter::emitRefOrConstant(llvm::Value *value) {
  if (isLocal(value) || isGlobal(value)) {
    return emitRef(value);
  }

  return emitConstant(value);
}

CPGProtoNode *CPGEmitter::emitRef(llvm::Value *value) {
  assert((isLocal(value) || isGlobal(value) || isConstExpr(value)) &&
         "Cannot emit reference to a non-variable");

  if (isConstExpr(value)) {
    return emitConstantExpr(llvm::cast<llvm::ConstantExpr>(value));
  }

  CPGProtoNode *valueRef = builder.identifierNode();
  (*valueRef) //
      .setName(value->getName())
      .setCode(value->getName())
      .setTypeFullName(getTypeName(value->getType()));

  if (!isGlobal(value)) {
    builder.connectREF(valueRef, getLocal(value));
  }
  resolveConnections(valueRef, {});
  return valueRef;
}

// ConstantInt, ConstantFP, ConstantAggregateZero,

// TODO: Think of a better solution
// At the bitcode level we don't know whether a number is signed or not
// Also we don't know if it's a boolean, or an arbitrary number
static std::string constantIntToString(const llvm::APInt &constant) {
  // assume boolean if it's one bit
  if (constant.getBitWidth() == 1) {
    if (constant.isAllOnesValue()) {
      return std::string("true");
    }
    return std::string("false");
  }
  bool printAsSigned = constant.isNegative();
  return constant.toString(10, printAsSigned);
}

CPGProtoNode *CPGEmitter::emitConstant(llvm::Value *value) {
  if (auto constantInt = llvm::dyn_cast<llvm::ConstantInt>(value)) {
    const llvm::APInt &constant = constantInt->getValue();
    CPGProtoNode *literalNode = builder.literalNode();
    (*literalNode) //
        .setTypeFullName(getTypeName(constantInt->getType()))
        .setCode(constantIntToString(constant));
    resolveConnections(literalNode, {});
    return literalNode;
  }

  if (auto constantExpr = llvm::dyn_cast<llvm::ConstantExpr>(value)) {
    return emitConstantExpr(constantExpr);
  }

  if (auto function = llvm::dyn_cast<llvm::Function>(value)) {
    // TODO: handle functions without a name
    CPGProtoNode *methodRef = builder.methodRef();
    (*methodRef) //
        .setCode(function->getName())
        .setMethodInstFullName(function->getName())
        .setMethodFullName(function->getName());
    resolveConnections(methodRef, {});
    return methodRef;
  }

  // TODO: Consider commenting out the following cases and just using the fallback

  if (llvm::isa<llvm::ConstantPointerNull>(value)) {
    CPGProtoNode *literalNode = builder.literalNode();
    (*literalNode) //
        .setTypeFullName(getTypeName(value->getType()))
        .setCode("nullptr");
    resolveConnections(literalNode, {});
    return literalNode;
  }

  if (auto constantFP = llvm::dyn_cast<llvm::ConstantFP>(value)) {
    const llvm::APFloat &constant = constantFP->getValueAPF();
    CPGProtoNode *literalNode = builder.literalNode();
    (*literalNode) //
        .setTypeFullName(getTypeName(constantFP->getType()))
        .setCode(std::to_string(constant.convertToDouble()));
    resolveConnections(literalNode, {});
    return literalNode;
  }

  if (auto constantAggregateZero = llvm::dyn_cast<llvm::ConstantAggregateZero>(value)) {
    CPGProtoNode *literalNode = builder.literalNode();
    (*literalNode) //
        .setTypeFullName(getTypeName(constantAggregateZero->getType()))
        .setCode("zero initialized");
    resolveConnections(literalNode, {});
    return literalNode;
  }

  if (auto constVec = llvm::dyn_cast<llvm::ConstantVector>(value)) {
    CPGProtoNode *literalNode = builder.literalNode();
    (*literalNode) //
        .setTypeFullName(getTypeName(value->getType()))
        .setCode(valueToString(value));
    resolveConnections(literalNode, {});
    return literalNode;
  }

  if (auto undef = llvm::dyn_cast<llvm::UndefValue>(value)) {
    CPGProtoNode *literalNode = builder.literalNode();
    (*literalNode) //
        .setTypeFullName(getTypeName(undef->getType()))
        .setCode("undef");
    resolveConnections(literalNode, {});
    return literalNode;
  }

  // look it up in llvm/IR/Value.def
  llvm::errs() << "Cannot handle constant yet: " << *value << " with ValueID "
               << value->getValueID();
  CPGProtoNode *literalNode = builder.literalNode();
  (*literalNode) //
      .setTypeFullName(getTypeName(value->getType()))
      .setCode(valueToString(value));
  resolveConnections(literalNode, {});
  return literalNode;
}

CPGProtoNode *CPGEmitter::emitConstant(unsigned int c) {
  CPGProtoNode *literalNode = builder.literalNode();
  (*literalNode) //
      .setTypeFullName("i32")
      .setCode(std::to_string(c));
  resolveConnections(literalNode, {});
  return literalNode;
}

CPGProtoNode *CPGEmitter::emitConstantExpr(llvm::ConstantExpr *constantExpr) {
  llvm::Instruction *constInstruction = constantExpr->getAsInstruction();
  if (auto gep = llvm::dyn_cast<llvm::GetElementPtrInst>(constInstruction)) {
    return emitGEP(gep);
  }
  if (auto cast = llvm::dyn_cast<llvm::CastInst>(constInstruction)) {
    return emitCast(cast);
  }
  if (auto binary = llvm::dyn_cast<llvm::BinaryOperator>(constInstruction)) {
    return emitBinaryCall(binary);
  }
  if (auto cmp = llvm::dyn_cast<llvm::CmpInst>(constInstruction)) {
    return emitCmpCall(cmp);
  }
  llvm::errs() << "Cannot handle constant expression yet: " << *constInstruction << "\n";
  CPGProtoNode *unhandled = builder.unknownNode();
  resolveConnections(unhandled, {});

  return unhandled;
}

CPGProtoNode *CPGEmitter::emitLocalVariable(const llvm::Value *variable, size_t order) {
  CPGProtoNode *local = builder.localVariableNode();
  (*local) //
      .setName(variable->getName())
      .setCode(variable->getName())
      .setTypeFullName(getTypeName(variable->getType()))
      .setOrder(order);
  return local;
}

CPGProtoNode *CPGEmitter::emitFunctionArgument(const llvm::Value *argument, size_t order) {
  CPGProtoNode *parameterInNode = builder.methodParameterInNode();
  (*parameterInNode) //
      .setName(argument->getName())
      .setCode(argument->getName())
      .setTypeFullName(getTypeName(argument->getType()))
      .setEvaluationStrategy("EVAL")
      .setOrder(order);
  return parameterInNode;
}

CPGProtoNode *CPGEmitter::emitAssignCall(const llvm::Value *value, CPGProtoNode *lhs,
                                         CPGProtoNode *rhs) {
  CPGProtoNode *assignCall = builder.functionCallNode();
  (*assignCall) //
      .setName("=")
      .setCode("=")
      .setTypeFullName(getTypeName(value->getType()))
      .setMethodInstFullName("=")
      .setMethodFullName("=")
      .setSignature("xxx")
      .setDispatchType("STATIC_DISPATCH");

  resolveConnections(assignCall, { lhs, rhs });
  return assignCall;
}

CPGProtoNode *CPGEmitter::emitAllocaCall(const llvm::Value *value) {
  CPGProtoNode *allocaCall = builder.functionCallNode();
  (*allocaCall) //
      .setName("alloca")
      .setCode(valueToString(value))
      .setTypeFullName(getTypeName(value->getType()))
      .setMethodInstFullName("alloca")
      .setMethodFullName("alloca")
      .setSignature("xxx")
      .setDispatchType("STATIC_DISPATCH");

  resolveConnections(allocaCall, {});
  return allocaCall;
}

CPGProtoNode *CPGEmitter::emitIndirectionCall(const llvm::Value *value, CPGProtoNode *pointerRef) {
  CPGProtoNode *indirectionCall = builder.functionCallNode();
  (*indirectionCall) //
      .setName("store")
      .setCode("store")
      .setTypeFullName(getTypeName(value->getType()))
      .setMethodInstFullName("store")
      .setMethodFullName("store")
      .setSignature("xxx")
      .setDispatchType("STATIC_DISPATCH");
  resolveConnections(indirectionCall, { pointerRef });
  return indirectionCall;
}

CPGProtoNode *CPGEmitter::emitDereference(llvm::Value *value) {
  CPGProtoNode *addressRef = emitRef(value);
  addressRef->setOrder(1).setArgumentIndex(1);
  CPGProtoNode *dereferenceCall = builder.functionCallNode();
  (*dereferenceCall) //
      .setName("load")
      .setCode("load")
      .setMethodInstFullName("load")
      .setMethodFullName("load")
      .setSignature("xxx")
      .setDispatchType("STATIC_DISPATCH")
      .setTypeFullName(getTypeName(value->getType()));

  resolveConnections(dereferenceCall, { addressRef });
  return dereferenceCall;
}

CPGProtoNode *CPGEmitter::emitBinaryCall(const llvm::BinaryOperator *binary) {
  CPGProtoNode *binCall = builder.functionCallNode();
  (*binCall) //
      .setName(binary->getOpcodeName())
      .setCode(binary->getOpcodeName())
      .setTypeFullName(getTypeName(binary->getType()))
      .setMethodInstFullName(binary->getOpcodeName())
      .setMethodFullName(binary->getOpcodeName())
      .setSignature("xxx")
      .setDispatchType("STATIC_DISPATCH");

  assert(binary->getNumOperands() == 2);
  CPGProtoNode *lhs = emitRefOrConstant(binary->getOperand(0));
  CPGProtoNode *rhs = emitRefOrConstant(binary->getOperand(1));

  resolveConnections(binCall, { lhs, rhs });
  return binCall;
}

CPGProtoNode *CPGEmitter::emitCmpCall(const llvm::CmpInst *comparison) {
  auto cmpCall = builder.functionCallNode();
  std::string name(comparison->getOpcodeName());
  name += '_';
  name += llvm::CmpInst::getPredicateName(comparison->getPredicate());
  (*cmpCall) //
      .setName(name)
      .setCode(name)
      .setTypeFullName(getTypeName(comparison->getType()))
      .setMethodInstFullName(name)
      .setMethodFullName(name)
      .setSignature("xxx")
      .setDispatchType("STATIC_DISPATCH");

  assert(comparison->getNumOperands() == 2);
  auto lhs = emitRefOrConstant(comparison->getOperand(0));
  auto rhs = emitRefOrConstant(comparison->getOperand(1));

  resolveConnections(cmpCall, { lhs, rhs });
  return cmpCall;
}

CPGProtoNode *CPGEmitter::emitCast(const llvm::CastInst *instruction) {
  CPGProtoNode *castCall = builder.functionCallNode();
  std::string name(instruction->getOpcodeName());
  (*castCall) //
      .setName(name)
      .setCode(name)
      .setTypeFullName(getTypeName(instruction->getDestTy()))
      .setMethodInstFullName(name)
      .setMethodFullName(name)
      .setSignature("xxx")
      .setDispatchType("STATIC_DISPATCH");

  CPGProtoNode *cast = emitRefOrConstant(instruction->getOperand(0));

  resolveConnections(castCall, { cast });
  return castCall;
}

CPGProtoNode *CPGEmitter::emitSelect(llvm::SelectInst *instruction) {
  CPGProtoNode *selectCall = builder.functionCallNode();
  std::string name(instruction->getOpcodeName());
  (*selectCall) //
      .setName(name)
      .setCode(name)
      .setTypeFullName(getTypeName(instruction->getType()))
      .setMethodInstFullName(name)
      .setMethodFullName(name)
      .setSignature("xxx")
      .setDispatchType("STATIC_DISPATCH");

  // TODO: at a first glance it is unclear if on these can by null or not
  CPGProtoNode *conditionValue = emitRefOrConstant(instruction->getCondition());
  CPGProtoNode *trueValue = emitRefOrConstant(instruction->getTrueValue());
  CPGProtoNode *falseValue = emitRefOrConstant(instruction->getFalseValue());

  resolveConnections(selectCall, { conditionValue, trueValue, falseValue });
  return selectCall;
}

//  This function yields a type for each index using the following rules:
//  *t => t
//  struct S { t1, t2, ..., tN } => S[index] => t1 or t2 or ... tN depending on the index' value
//  [ t ] => t
//  TODO: check sized structs
static llvm::Type *nextIndexType(llvm::Type *type, llvm::Value *index) {
  if (auto pointerType = llvm::dyn_cast<llvm::PointerType>(type)) {
    return pointerType->getElementType();
  }
  if (auto structType = llvm::dyn_cast<llvm::StructType>(type)) {
    auto *constantIndex = llvm::cast<llvm::ConstantInt>(index);
    assert(constantIndex->getValue().getNumWords() == 1 &&
           "Struct members always indexed using i32");
    int32_t intIndex = *constantIndex->getValue().getRawData();
    assert(intIndex < structType->getNumElements());
    return structType->getElementType(intIndex);
  }

  if (auto arrayType = llvm::dyn_cast<llvm::ArrayType>(type)) {
    return arrayType->getElementType();
  }

  if (auto vectorType = llvm::dyn_cast<llvm::VectorType>(type)) {
    return vectorType->getElementType();
  }

  llvm::errs() << *type << " || " << *index << "\n";

  llvm_unreachable("Cannot handle the type above yet");
}

static llvm::Type *nextIndexType(llvm::Type *type, unsigned int index) {
  if (auto structType = llvm::dyn_cast<llvm::StructType>(type)) {
    assert(index < structType->getNumElements());
    return structType->getElementType(index);
  }
  if (auto arrayType = llvm::dyn_cast<llvm::ArrayType>(type)) {
    return arrayType->getElementType();
  }

  llvm::errs() << *type << " || " << index << "\n";

  llvm_unreachable("Cannot handle the type above yet");
}

CPGProtoNode *CPGEmitter::emitGEP(const llvm::GetElementPtrInst *instruction) {
  //
  //  It is highly recommended to read these documents to get better understanding:
  //
  //   - https://llvm.org/docs/LangRef.html#getelementptr-instruction
  //   - http://llvm.org/docs/GetElementPtr.html
  //
  //  The following documentation is just a brief note on how we emit CPG for the GEP instruction.
  //
  //  GEP, or getelementptr, computes an address of a struct member or of a vector element.
  //  It has the following structure:
  //
  //    %x = getelementptr %ptr, index_0 [, index_1, ..., index_N]
  //
  //  which is equivalent to the following:
  //
  //    x = ptr[index_0][index_1] ... [index_N]
  //
  //  Each index is either a constant or a value.
  //  In case of a struct an index_X points to a struct member.
  //
  //  For each index we emit either an index_access (for vector accesses) or
  //  a member_access (for struct accesses).
  //  The very first index is always an index_access and its type is the type of (*ptr).
  //  Every other index is either an index_access or a member_access depending on a type.
  //  The type of each index is determined using the nextIndexType function.
  //  The type of the last access is always a pointer.
  //
  //  The CPG is built from left to right for each index, and the AST relation is built bottom-up:
  //  each CPG node becomes a child of the next CPG node.
  //
  //  The GEP instruction has the following operands:
  //    0: pointer
  //    1: first index, always present
  //    2..N: any other additional index
  //
  //  TODO: handle vectors of addresses, i.e. getelementptr yielding [ i32*, i32*, i32*, ... ]
  //  instead of a single address
  //

  llvm::Value *element = instruction->getOperand(0);
  llvm::Value *index = instruction->getOperand(1);

  llvm::Type *indexType = nextIndexType(element->getType(), index);
  if (instruction->getNumIndices() == 1) {
    indexType = instruction->getType();
  }

  CPGProtoNode *access = emitGEPAccess(indexType, index, false);
  CPGProtoNode *lhs = emitRefOrConstant(element);
  CPGProtoNode *rhs = emitRefOrConstant(index);

  resolveConnections(access, { lhs, rhs });

  // Skipping the pointer operand and the first index
  for (size_t i = 2; i < instruction->getNumOperands(); i++) {
    bool isStruct = indexType->isStructTy();
    index = instruction->getOperand(i);
    indexType = nextIndexType(indexType, index);
    if (i == instruction->getNumOperands() - 1) {
      indexType = instruction->getType();
    }

    lhs = access;
    rhs = emitRefOrConstant(index);
    access = emitGEPAccess(indexType, index, isStruct);
    resolveConnections(access, { lhs, rhs });
  }

  return access;
}

CPGProtoNode *CPGEmitter::emitInsertValue(llvm::InsertValueInst *instruction) {
  //  cf emitGEP

  llvm::Value *res = instruction->getAggregateOperand();
  llvm::Value *val = instruction->getInsertedValueOperand();
  llvm::Type *indexType = res->getType();
  CPGProtoNode *agg = emitRefOrConstant(res);
  auto indices = instruction->getIndices();

  for (unsigned int i = 0; i < instruction->getNumIndices(); i++) {
    unsigned int index = indices[i];
    CPGProtoNode *idxCpg = emitConstant(index);
    bool isStruct = indexType->isStructTy();
    indexType = nextIndexType(indexType, index);
    CPGProtoNode *access = emitInsertAccess(indexType, index, isStruct);
    resolveConnections(access, { agg, idxCpg });
    agg = access;
  }
  // We have walked the indexing chain, and now need to emit the final insert.
  CPGProtoNode *call = builder.functionCallNode();
  (*call) //
      .setName("insertValue")
      .setCode("insertValue")
      .setTypeFullName(getTypeName(res->getType()))
      .setMethodInstFullName("insertValue")
      .setMethodFullName("insertValue")
      .setSignature("xxx")
      .setDispatchType("STATIC_DISPATCH");

  CPGProtoNode *inserted = emitRefOrConstant(val);
  resolveConnections(call, { agg, inserted });

  return call;
}

CPGProtoNode *CPGEmitter::emitExtractValue(llvm::ExtractValueInst *instruction) {
  llvm::Value *from = instruction->getAggregateOperand();
  llvm::Type *indexType = from->getType();
  CPGProtoNode *agg = emitRefOrConstant(from);
  auto indices = instruction->getIndices();

  for (unsigned int i = 0; i < instruction->getNumIndices(); i++) {
    unsigned int index = indices[i];
    CPGProtoNode *idx_cpg = emitConstant(index);
    bool isStruct = indexType->isStructTy();
    indexType = nextIndexType(indexType, index);
    CPGProtoNode *access = emitExtract(indexType, index, isStruct);
    resolveConnections(access, { agg, idx_cpg });
    agg = access;
  }

  //  cf emitInsertValue
  return agg;
}

CPGProtoNode *CPGEmitter::emitGEPAccess(const llvm::Type *type, llvm::Value *index,
                                        bool memberAccess) {
  std::string name("index_access");
  if (memberAccess) {
    name = "member_access";
  }
  CPGProtoNode *call = builder.functionCallNode();
  (*call) //
      .setName(name)
      .setCode(name)
      .setTypeFullName(getTypeName(type))
      .setMethodInstFullName(name)
      .setMethodFullName(name)
      .setSignature("xxx")
      .setDispatchType("STATIC_DISPATCH");

  return call;
}

CPGProtoNode *CPGEmitter::emitInsertAccess(const llvm::Type *type, unsigned int idx,
                                           bool memberAccess) {
  std::string name("insertValue_indexShift");
  if (memberAccess) {
    name = "insertValue_memberSelect";
  }
  CPGProtoNode *call = builder.functionCallNode();
  (*call) //
      .setName(name)
      .setCode(name)
      .setTypeFullName(getTypeName(type))
      .setMethodInstFullName(name)
      .setMethodFullName(name)
      .setSignature("xxx")
      .setDispatchType("STATIC_DISPATCH");

  return call;
}

CPGProtoNode *CPGEmitter::emitExtract(const llvm::Type *type, unsigned int idx, bool memberAccess) {
  std::string name("ExtractValue_index");
  if (memberAccess) {
    name = "ExtractValue_member";
  }
  CPGProtoNode *call = builder.functionCallNode();
  (*call) //
      .setName(name)
      .setCode(name)
      .setTypeFullName(getTypeName(type))
      .setMethodInstFullName(name)
      .setMethodFullName(name)
      .setSignature("xxx")
      .setDispatchType("STATIC_DISPATCH");

  return call;
}

CPGProtoNode *CPGEmitter::emitUnaryOperator(const llvm::UnaryOperator *instruction) {
  CPGProtoNode *fnegCall = builder.functionCallNode();
  std::string name(instruction->getOpcodeName());
  (*fnegCall) //
      .setName(name)
      .setCode(name)
      .setTypeFullName(getTypeName(instruction->getType()))
      .setMethodInstFullName(name)
      .setMethodFullName(name)
      .setSignature("xxx")
      .setDispatchType("STATIC_DISPATCH");

  CPGProtoNode *argument = emitRefOrConstant(instruction->getOperand(0));

  resolveConnections(fnegCall, { argument });
  return fnegCall;
}

CPGProtoNode *CPGEmitter::emitFunctionCall(const llvm::CallInst *instruction) {
  if (!instruction->getCalledFunction()) {
    CPGProtoNode *callNode = builder.functionCallNode();
    std::string name("fptr");
    (*callNode) //
        .setName(name)
        .setCode(name)
        .setTypeFullName(getTypeName(instruction->getType()))
        .setMethodInstFullName(name)
        .setMethodFullName(name)
        .setSignature("xxx")
        .setDispatchType("DYNAMIC_DISPATCH");

    CPGProtoNode *receiver = emitRefOrConstant(instruction->getCalledOperand());
    builder.connectReceiver(callNode, receiver);

    std::vector<CPGProtoNode *> children({ receiver });
    for (const llvm::Use &argument : instruction->args()) {
      CPGProtoNode *arg = emitRefOrConstant(argument.get());
      children.push_back(arg);
    }
    // TODO: Receiver should not be connected via AST edge
    resolveConnections(callNode, children);
    receiver->setArgumentIndex(0);
    return callNode;
  }

  CPGProtoNode *call = builder.functionCallNode();
  std::string name(instruction->getCalledFunction()->getName());
  (*call) //
      .setName(name)
      .setCode(name)
      .setTypeFullName(getTypeName(instruction->getType()))
      .setMethodInstFullName(name)
      .setMethodFullName(name)
      .setSignature("xxx")
      .setDispatchType("STATIC_DISPATCH");

  std::vector<CPGProtoNode *> children;
  for (const llvm::Use &argument : instruction->args()) {
    CPGProtoNode *arg = emitRefOrConstant(argument.get());
    children.push_back(arg);
  }

  resolveConnections(call, children);
  return call;
}

CPGProtoNode *CPGEmitter::emitNoop() {
  CPGProtoNode *noop = builder.unknownNode();
  noop->setCode("noop");
  noop->setEntry(noop->getID());
  return noop;
}

CPGProtoNode *CPGEmitter::emitUnhandled() {
  CPGProtoNode *unhandled = builder.unknownNode();
  unhandled->setCode("unhandled");
  unhandled->setEntry(unhandled->getID());
  return unhandled;
}

const CPGProtoNode *CPGEmitter::getLocal(const llvm::Value *value) const {
  assert(isLocal(value) && "Local was not emitted");
  return locals.find(value)->second;
}

bool CPGEmitter::isLocal(const llvm::Value *value) const {
  return locals.count(value) != 0;
}

bool CPGEmitter::isGlobal(const llvm::Value *value) const {
  return globals.count(value) != 0;
}

bool CPGEmitter::isConstExpr(const llvm::Value *value) const {
  return llvm::isa<llvm::ConstantExpr>(value);
}

void CPGEmitter::resolveConnections(CPGProtoNode *parent, std::vector<CPGProtoNode *> children) {
  resolveCFGConnections(parent, children);
  resolveASTConnections(parent, children);
}

void CPGEmitter::resolveASTConnections(CPGProtoNode *parent, std::vector<CPGProtoNode *> children) {
  for (size_t i = 0; i < children.size(); i++) {
    CPGProtoNode *current = children[i];
    current->setOrder(i + 1).setArgumentIndex(i + 1);
    builder.connectAST(parent, current);
  }
}

void CPGEmitter::resolveCFGConnections(CPGProtoNode *parent, std::vector<CPGProtoNode *> children) {
  if (children.empty()) {
    parent->setEntry(parent->getID());
    return;
  }

  parent->setEntry(children.front()->getEntry());
  builder.connectCFG(children.back()->getID(), parent->getID());

  for (size_t i = 0; i < children.size() - 1; i++) {
    CPGProtoNode *current = children[i];
    CPGProtoNode *next = children[i + 1];
    builder.connectCFG(current->getID(), next->getEntry());
  }
}

std::string CPGEmitter::getTypeName(const llvm::Type *type) {
  return typeEmitter.recordType(type, file.getGlobalNamespaceName());
}
