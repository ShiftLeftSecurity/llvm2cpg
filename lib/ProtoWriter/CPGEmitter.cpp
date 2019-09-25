#include "CPGEmitter.h"
#include "llvm2cpg/CPG/CPGMethod.h"

using namespace llvm2cpg;

static std::string typeToString(llvm::Type *type) {
  std::string typeName;
  llvm::raw_string_ostream stream(typeName);
  type->print(stream);
  stream.flush();
  return typeName;
}

static std::string valueToString(const llvm::Value *value) {
  std::string name;
  llvm::raw_string_ostream stream(name);
  value->printAsOperand(stream);
  stream.flush();
  return name;
}

CPGEmitter::CPGEmitter(CPGProtoBuilder &builder) : builder(builder) {}

void CPGEmitter::emitMethod(const CPGMethod &method) {
  CPGProtoNode *methodNode = emitMethodNode(method);
  CPGProtoNode *methodReturnNode = emitMethodReturnNode(method);
  CPGProtoNode *methodBlock = emitMethodBlock(method);

  builder.connectAST(methodNode, methodBlock);
  builder.connectAST(methodNode, methodReturnNode);

  for (size_t localIndex = 0; localIndex < method.getLocalVariables().size(); localIndex++) {
    llvm::Value *variable = method.getLocalVariables()[localIndex];
    CPGProtoNode *local = emitLocalVariable(variable, localIndex);
    builder.connectAST(methodBlock, local);
    locals.insert(std::make_pair(variable, local));
  }

  for (size_t argIndex = 0; argIndex < method.getArguments().size(); argIndex++) {
    llvm::Value *argument = method.getArguments()[argIndex];
    CPGProtoNode *parameterInNode = emitFunctionArgument(argument, argIndex);
    builder.connectAST(methodNode, parameterInNode);
    locals.insert(std::make_pair(argument, parameterInNode));
  }

  std::unordered_map<llvm::Value *, CPGProtoNode *> topLevelNodes;
  std::vector<llvm::BranchInst *> unresolvedBranches;

  size_t topLevelOrder = 1;
  for (llvm::BasicBlock &basicBlock : method.getFunction()) {
    std::vector<CPGProtoNode *> nodes;
    for (llvm::Instruction &instruction : basicBlock) {
      // We cannot make CFG connections for these branches yet
      // So we collect them for later use
      if (auto branch = llvm::dyn_cast<llvm::BranchInst>(&instruction)) {
        unresolvedBranches.push_back(branch);
        continue;
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
  auto &basicBlocks = method.getFunction().getBasicBlockList();

  llvm::Instruction *firstInstruction = &basicBlocks.front().getInstList().front();
  assert(topLevelNodes.count(firstInstruction) != 0);
  CPGProtoNode *head = topLevelNodes.at(firstInstruction);
  builder.connectCFG(methodNode->getID(), head->getEntry());

  llvm::Instruction *lastInstruction = &basicBlocks.back().getInstList().back();
  assert(topLevelNodes.count(lastInstruction) != 0);
  CPGProtoNode *retNode = topLevelNodes.at(lastInstruction);
  builder.connectCFG(retNode->getID(), methodReturnNode->getID());

  // Connect CFG for unresolved branches
  for (llvm::BranchInst *branch : unresolvedBranches) {
    llvm::Value *sourceInstruction = nullptr;
    if (branch->isConditional()) {
      sourceInstruction = branch->getCondition();
    } else {
      // TODO: may yield nullptr if the building block is empty
      sourceInstruction = branch->getPrevNonDebugInstruction();
    }
    CPGProtoNode *source = topLevelNodes.at(sourceInstruction);
    for (size_t i = 0; i < branch->getNumSuccessors(); i++) {
      llvm::BasicBlock *successor = branch->getSuccessor(i);
      // TODO: won't work if the successor is an empty basic block
      CPGProtoNode *destination = topLevelNodes.at(&successor->front());
      builder.connectCFG(source->getID(), destination->getEntry());
    }
  }
}

CPGProtoNode *CPGEmitter::visitInstruction(llvm::Instruction &instruction) {
  llvm::errs() << "Cannot handle: " << instruction << ". Skipping.\n";
  return nullptr;
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
      .setASTParentFullName("global_namespace")
      .setIsExternal(method.isExternal())
      .setSignature(method.getSignature())
      .setOrder(0);
  return methodNode;
}

CPGProtoNode *CPGEmitter::emitMethodReturnNode(const CPGMethod &method) {
  std::string methodReturnType = typeToString(method.getReturnType());
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
  std::string methodReturnType = typeToString(method.getReturnType());
  (*blockNode) //
      .setOrder(0)
      .setTypeFullName(methodReturnType)
      .setCode("x + 42")
      .setArgumentIndex(0);
  return blockNode;
}

CPGProtoNode *CPGEmitter::emitRefOrConstant(const llvm::Value *value) {
  if (isLocal(value)) {
    return emitRef(value);
  }

  return emitConstant(value);
}

CPGProtoNode *CPGEmitter::emitRef(const llvm::Value *value) {
  assert(isLocal(value) && "Local was not emitted");

  CPGProtoNode *valueRef = builder.identifierNode();
  (*valueRef) //
      .setName(value->getName())
      .setCode(value->getName())
      .setTypeFullName(typeToString(value->getType()));
  builder.connectREF(valueRef, getLocal(value));
  resolveConnections(valueRef, {});
  return valueRef;
}

CPGProtoNode *CPGEmitter::emitConstant(const llvm::Value *value) {
  if (auto constantInt = llvm::dyn_cast<llvm::ConstantInt>(value)) {
    CPGProtoNode *literalNode = builder.literalNode();
    (*literalNode) //
        .setTypeFullName(typeToString(constantInt->getType()))
        .setCode(constantInt->getValue().toString(10, true));
    resolveConnections(literalNode, {});
    return literalNode;
  }

  llvm::errs() << "Cannot handle constant yet: " << *value << "\n";

  return builder.unknownNode();
}

CPGProtoNode *CPGEmitter::emitLocalVariable(const llvm::Value *variable, size_t order) {
  CPGProtoNode *local = builder.localVariableNode();
  (*local) //
      .setName(variable->getName())
      .setCode(variable->getName())
      .setTypeFullName(typeToString(variable->getType()))
      .setOrderAndIndex(order);
  return local;
}

CPGProtoNode *CPGEmitter::emitFunctionArgument(const llvm::Value *argument, size_t order) {
  CPGProtoNode *parameterInNode = builder.methodParameterInNode();
  (*parameterInNode) //
      .setName(argument->getName())
      .setCode(argument->getName())
      .setTypeFullName(typeToString(argument->getType()))
      .setEvaluationStrategy("EVAL")
      .setOrderAndIndex(order);
  return parameterInNode;
}

CPGProtoNode *CPGEmitter::emitAssignCall(const llvm::Value *value, CPGProtoNode *lhs,
                                         CPGProtoNode *rhs) {
  CPGProtoNode *assignCall = builder.functionCallNode();
  (*assignCall) //
      .setName("=")
      .setCode("=")
      .setTypeFullName(typeToString(value->getType()))
      .setMethodInstFullName("=")
      .setSignature("xxx")
      .setDispatchType("STATIC");

  resolveConnections(assignCall, { lhs, rhs });
  return assignCall;
}

CPGProtoNode *CPGEmitter::emitAllocaCall(const llvm::Value *value) {
  CPGProtoNode *allocaCall = builder.functionCallNode();
  (*allocaCall) //
      .setName("alloca")
      .setCode(valueToString(value))
      .setTypeFullName(typeToString(value->getType()))
      .setMethodInstFullName("alloca")
      .setSignature("xxx")
      .setDispatchType("STATIC");

  resolveConnections(allocaCall, {});
  return allocaCall;
}

CPGProtoNode *CPGEmitter::emitIndirectionCall(const llvm::Value *value, CPGProtoNode *pointerRef) {
  CPGProtoNode *indirectionCall = builder.functionCallNode();
  (*indirectionCall) //
      .setName("store")
      .setCode("store")
      .setTypeFullName(typeToString(value->getType()))
      .setMethodInstFullName("store")
      .setSignature("xxx")
      .setDispatchType("STATIC");
  resolveConnections(indirectionCall, { pointerRef });
  return indirectionCall;
}

CPGProtoNode *CPGEmitter::emitDereference(const llvm::Value *value) {
  CPGProtoNode *addressRef = emitRef(value);
  addressRef->setOrder(1).setArgumentIndex(1);
  CPGProtoNode *dereferenceCall = builder.functionCallNode();
  (*dereferenceCall) //
      .setName("load")
      .setCode("load")
      .setMethodInstFullName("load")
      .setSignature("xxx")
      .setDispatchType("STATIC")
      .setTypeFullName(typeToString(value->getType()));

  resolveConnections(dereferenceCall, { addressRef });
  return dereferenceCall;
}

CPGProtoNode *CPGEmitter::emitBinaryCall(const llvm::BinaryOperator *binary) {
  CPGProtoNode *binCall = builder.functionCallNode();
  (*binCall) //
      .setName(binary->getOpcodeName())
      .setCode(binary->getOpcodeName())
      .setTypeFullName(typeToString(binary->getType()))
      .setMethodInstFullName(binary->getOpcodeName())
      .setSignature("xxx")
      .setDispatchType("STATIC");

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
      .setTypeFullName(typeToString(comparison->getType()))
      .setMethodInstFullName(comparison->getOpcodeName())
      .setSignature("xxx")
      .setDispatchType("STATIC");

  assert(comparison->getNumOperands() == 2);
  auto lhs = emitRefOrConstant(comparison->getOperand(0));
  auto rhs = emitRefOrConstant(comparison->getOperand(1));

  resolveConnections(cmpCall, { lhs, rhs });
  return cmpCall;
}

const CPGProtoNode *CPGEmitter::getLocal(const llvm::Value *value) const {
  assert(isLocal(value) && "Local was not emitted");
  return locals.find(value)->second;
}

bool CPGEmitter::isLocal(const llvm::Value *value) const {
  return locals.count(value) != 0;
}

void CPGEmitter::resolveConnections(CPGProtoNode *parent, std::vector<CPGProtoNode *> children) {
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
  for (size_t i = 0; i < children.size(); i++) {
    CPGProtoNode *current = children[i];
    current->setOrder(i + 1).setArgumentIndex(i + 1);
    builder.connectAST(parent, current);
  }
}
