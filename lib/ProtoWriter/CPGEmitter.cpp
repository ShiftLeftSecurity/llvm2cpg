#include "CPGEmitter.h"
#include "CPGProtoBuilder.h"
#include "CPGProtoNode.h"
#include "CPGTypeEmitter.h"
#include "llvm2cpg/CPG/CPGFile.h"
#include "llvm2cpg/CPG/CPGMethod.h"
#include "llvm2cpg/CPG/CPGOperatorNames.h"
#include "llvm2cpg/Logger/CPGLogger.h"
#include <iomanip>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/InlineAsm.h>
#include <llvm/Support/JSON.h>
#include <sstream>

using namespace llvm2cpg;

static std::string valueToString(const llvm::Value *value) {
  std::string name;
  llvm::raw_string_ostream stream(name);
  value->printAsOperand(stream);
  return stream.str();
}

CPGEmitter::CPGEmitter(CPGLogger &logger, CPGProtoBuilder &builder, CPGTypeEmitter &typeEmitter,
                       const CPGFile &file)
    : logger(logger), builder(builder), typeEmitter(typeEmitter), file(file), lineNumber(0),
      columnNumber(0), inlineMD(0) {}

CPGProtoNode *CPGEmitter::emitMethod(const CPGMethod &method) {
  CPGProtoNode *methodNode = emitMethodNode(method);
  CPGProtoNode *methodReturnNode = emitMethodReturnNode(method);

  CPGProtoNode *methodBlock = emitMethodBlock(method);
  llvm::Function *fun = &method.getFunction();
  inlineMD = fun->getContext().getMDKindID("shiftleft.inline");

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
    return methodNode;
  }

  logger.logInfo(std::string("Emitting ") +
                 demangler.demangleFunctionName(&method.getFunction()).name);

  llvm::Module *module = method.getFunction().getParent();
  for (llvm::GlobalVariable &global : module->getGlobalList()) {
    globals.insert(&global);
  }
  for (llvm::GlobalAlias &alias : module->getAliasList()) {
    globals.insert(&alias);
  }

  for (size_t localIndex = 0; localIndex < method.getLocalVariables().size(); localIndex++) {
    llvm::Value *variable = method.getLocalVariables()[localIndex];
    CPGProtoNode *local = emitLocalVariable(variable, localIndex);
    builder.connectAST(methodBlock, local);
    locals.insert(std::make_pair(variable, local));
  }

  std::unordered_map<const llvm::BasicBlock *, CPGProtoNode *> entryPoints;
  std::vector<CPGProtoNode *> exitPoints;

  std::vector<const llvm::Instruction *> unresolvedTerminators;

  size_t topLevelOrder = 1;
  for (llvm::BasicBlock &basicBlock : method.getFunction()) {
    std::vector<CPGProtoNode *> nodes;
    for (llvm::Instruction &instruction : basicBlock) {
      updateLineInfo(&instruction);
      // We cannot make CFG connections for the terminators yet
      // So we collect them for later use
      if (auto branch = llvm::dyn_cast<llvm::BranchInst>(&instruction)) {
        unresolvedTerminators.push_back(branch);
      }
      if (auto switchInst = llvm::dyn_cast<llvm::SwitchInst>(&instruction)) {
        unresolvedTerminators.push_back(switchInst);
      }
      if (instruction.getMetadata(inlineMD)) {
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
    }

    if (nodes.empty()) {
      nodes.push_back(emitNoop());
    }
    entryPoints.insert(std::make_pair(&basicBlock, nodes.front()));
    exitPoints.push_back(nodes.back());

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

  // Connect CFG: method -> method body
  builder.connectCFG(methodNode->getID(),
                     entryPoints.at(&(method.getFunction().getEntryBlock()))->getEntry());

  // Connect terminators in second pass
  int i = 0;
  for (llvm::BasicBlock &basicBlock : method.getFunction()) {
    llvm::Instruction *instruction = basicBlock.getTerminator();
    CPGProtoNode *node = exitPoints.at(i);
    if (llvm::dyn_cast<llvm::ReturnInst>(instruction)) {
      builder.connectCFG(node->getID(), methodReturnNode->getID());
    } else {
      int numSuccs = instruction->getNumSuccessors();
      if (!(llvm::isa<llvm::BranchInst>(instruction) || llvm::isa<llvm::SwitchInst>(instruction) ||
            llvm::isa<llvm::IndirectBrInst>(instruction) ||
            llvm::isa<llvm::UnreachableInst>(instruction) ||
            llvm::isa<llvm::ResumeInst>(instruction)
#if LLVM_VERSION_MAJOR >= 9
            || llvm::isa<llvm::CallBrInst>(instruction)
#endif
            || llvm::isa<llvm::InvokeInst>(instruction))) {
        logger.logWarning(std::string("Cannot handle terminator: ") + valueToString(instruction) +
                          std::string(" of ValueID: ") + std::to_string(instruction->getValueID()) +
                          "\n");
      }
      for (unsigned int j = 0; j < numSuccs; j++) {
        builder.connectCFG(node->getID(), entryPoints.at(instruction->getSuccessor(j))->getEntry());
      }
    }
    i++;
  }

  return methodNode;
}

CPGProtoNode *CPGEmitter::visitInstruction(llvm::Instruction &instruction) {
  if (!(llvm::isa<llvm::FenceInst>(instruction) || llvm::isa<llvm::LandingPadInst>(instruction) ||
        llvm::isa<llvm::ResumeInst>(instruction) || llvm::isa<llvm::CatchReturnInst>(instruction) ||
        llvm::isa<llvm::CatchSwitchInst>(instruction) ||
        llvm::isa<llvm::CatchPadInst>(instruction) ||
        llvm::isa<llvm::CleanupPadInst>(instruction) ||
        llvm::isa<llvm::CleanupReturnInst>(instruction))) { /*llvm::VAArgInst*/
    logger.logWarning(std::string("Cannot handle instruction: ") + valueToString(&instruction) +
                      std::string(" of ValueID: ") + std::to_string(instruction.getValueID()) +
                      "\n");
  }
  if (instruction.getType()->isVoidTy()) {
    return emitUnhandledCall(&instruction);
  } else {
    return emitAssignCall(
        instruction.getType(), emitRefOrConstant(&instruction), emitUnhandledCall(&instruction));
  }
}

CPGProtoNode *CPGEmitter::emitUnhandledCall(llvm::Instruction *instruction) {
  std::string name(instruction->getOpcodeName());
  return resolveConnections(emitGenericOp(name, name, getTypeName(instruction->getType()), "ANY"),
                            {});
}

CPGProtoNode *CPGEmitter::visitAllocaInst(llvm::AllocaInst &instruction) {
  return nullptr;
}

CPGProtoNode *CPGEmitter::visitDbgVariableIntrinsic(llvm::DbgVariableIntrinsic &instruction) {
  return nullptr;
}

CPGProtoNode *CPGEmitter::visitStoreInst(llvm::StoreInst &instruction) {
  llvm::Value *value = instruction.getValueOperand();
  llvm::Value *pointer = instruction.getPointerOperand();
  return emitAssignCall(value->getType(),
                        emitIndirectionCall(value->getType(), emitRefOrConstant(pointer)),
                        emitRefOrConstant(value));
}

CPGProtoNode *CPGEmitter::visitLoadInst(llvm::LoadInst &instruction) {
  return emitAssignCall(
      instruction.getType(), emitRefOrConstant(&instruction), emitDereference(&instruction));
}

CPGProtoNode *CPGEmitter::visitBinaryOperator(llvm::BinaryOperator &instruction) {
  return emitAssignCall(
      instruction.getType(), emitRefOrConstant(&instruction), emitBinaryCall(&instruction));
}

CPGProtoNode *CPGEmitter::visitCmpInst(llvm::CmpInst &instruction) {
  return emitAssignCall(
      instruction.getType(), emitRefOrConstant(&instruction), emitCmpCall(&instruction));
}

CPGProtoNode *CPGEmitter::visitCastInst(llvm::CastInst &instruction) {
  return emitAssignCall(
      instruction.getType(), emitRefOrConstant(&instruction), emitCast(&instruction));
}

CPGProtoNode *CPGEmitter::visitSelectInst(llvm::SelectInst &instruction) {
  return emitAssignCall(
      instruction.getType(), emitRefOrConstant(&instruction), emitSelect(&instruction));
}

CPGProtoNode *CPGEmitter::visitGetElementPtrInst(llvm::GetElementPtrInst &instruction) {
  return emitAssignCall(
      instruction.getType(), emitRefOrConstant(&instruction), emitGEP(&instruction));
}

CPGProtoNode *CPGEmitter::visitExtractValueInst(llvm::ExtractValueInst &instruction) {
  return emitAssignCall(
      instruction.getType(), emitRefOrConstant(&instruction), emitExtractValue(&instruction));
}

CPGProtoNode *CPGEmitter::visitUnaryOperator(llvm::UnaryOperator &instruction) {
  return emitAssignCall(
      instruction.getType(), emitRefOrConstant(&instruction), emitUnaryOperator(&instruction));
}

CPGProtoNode *CPGEmitter::visitCallBase(llvm::CallBase &instruction) {
  if (instruction.getFunctionType()->getReturnType()->isVoidTy()) {
    return emitFunctionCall(&instruction);
  }
  return emitAssignCall(
      instruction.getType(), emitRefOrConstant(&instruction), emitFunctionCall(&instruction));
}

CPGProtoNode *CPGEmitter::visitAtomicRMWInst(llvm::AtomicRMWInst &instruction) {
  return emitAssignCall(
      instruction.getType(), emitRefOrConstant(&instruction), emitAtomicRMW(&instruction));
}

CPGProtoNode *CPGEmitter::emitAtomicRMW(llvm::AtomicRMWInst *instruction) {
  std::string name(atomicOperatorName(instruction));
  return resolveConnections(
      emitGenericOp(name, name, getTypeName(instruction->getType()), "ANY (ANY, ANY)"),
      { emitRefOrConstant(instruction->getPointerOperand()),
        emitRefOrConstant(instruction->getValOperand()) });
}

CPGProtoNode *CPGEmitter::visitAtomicCmpXchgInst(llvm::AtomicCmpXchgInst &instruction) {
  return emitAssignCall(
      instruction.getType(), emitRefOrConstant(&instruction), emitAtomicCmpXchg(&instruction));
}

CPGProtoNode *CPGEmitter::emitAtomicCmpXchg(llvm::AtomicCmpXchgInst *instruction) {
  std::string name("<operator>.cmpxchg");
  return resolveConnections(
      emitGenericOp(name, name, getTypeName(instruction->getType()), "{ANY, i1} (ANY, ANY, ANY)"),
      { emitRefOrConstant(instruction->getPointerOperand()),
        emitRefOrConstant(instruction->getCompareOperand()),
        emitRefOrConstant(instruction->getNewValOperand()) });
}

CPGProtoNode *CPGEmitter::visitExtractElementInst(llvm::ExtractElementInst &instruction) {
  return emitAssignCall(
      instruction.getType(), emitRefOrConstant(&instruction), emitExtractElement(&instruction));
}

CPGProtoNode *CPGEmitter::emitExtractElement(llvm::ExtractElementInst *instruction) {
  return resolveConnections(emitGenericOp("<operator>.computedMemberAccess",
                                          "extractelement",
                                          getTypeName(instruction->getType()),
                                          "ANY (ANY)"),
                            { emitRefOrConstant(instruction->getVectorOperand()),
                              emitRefOrConstant(instruction->getIndexOperand()) });
}

CPGProtoNode *CPGEmitter::visitInsertElementInst(llvm::InsertElementInst &instruction) {
  return emitAssignCall(
      instruction.getType(), emitRefOrConstant(&instruction), emitInsertElement(&instruction));
}

CPGProtoNode *CPGEmitter::visitInsertValueInst(llvm::InsertValueInst &instruction) {
  return emitAssignCall(
      instruction.getType(), emitRefOrConstant(&instruction), emitInsertValue(&instruction));
}

CPGProtoNode *CPGEmitter::visitShuffleVectorInst(llvm::ShuffleVectorInst &instruction) {
  return emitAssignCall(
      instruction.getType(), emitRefOrConstant(&instruction), emitShuffleVector(&instruction));
}

CPGProtoNode *CPGEmitter::emitShuffleVector(llvm::ShuffleVectorInst *instruction) {
  return resolveConnections(emitGenericOp("<operator>.shufflevector",
                                          "<operator>.shufflevector",
                                          getTypeName(instruction->getType()),
                                          "ANY (ANY, ANY, ANY)"),
                            { emitRefOrConstant(instruction->getOperand(0)),
                              emitRefOrConstant(instruction->getOperand(1)),
                              emitRefOrConstant(instruction->getOperand(2)) });
}

CPGProtoNode *CPGEmitter::visitPHINode(llvm::PHINode &instruction) {
  logger.uiFatal("PHI nodes should be destructed before CPG emission");
  return nullptr;
}

CPGProtoNode *CPGEmitter::visitBranchInst(llvm::BranchInst &instruction) {
  return nullptr;
}

CPGProtoNode *CPGEmitter::visitSwitchInst(llvm::SwitchInst &instruction) {
  return nullptr;
}

CPGProtoNode *CPGEmitter::visitIndirectBrInst(llvm::IndirectBrInst &instruction) {
  return nullptr;
}

CPGProtoNode *CPGEmitter::visitUnreachableInst(llvm::UnreachableInst &instruction) {
  CPGProtoNode *node = emitUnhandled();
  node->setCode("unreachable");
  return node;
}

CPGProtoNode *CPGEmitter::visitReturnInst(llvm::ReturnInst &instruction) {
  CPGProtoNode *returnNode = builder.returnNode();
  returnNode->setCode("return");
  setLineInfo(returnNode);

  std::vector<CPGProtoNode *> children;
  if (llvm::Value *retValue = instruction.getReturnValue()) {
    CPGProtoNode *retValueNode = emitRefOrConstant(retValue);
    children.push_back(retValueNode);
  }
  resolveConnections(returnNode, children);
  return returnNode;
}

CPGProtoNode *CPGEmitter::emitMethodNode(const CPGMethod &method) {
  const DemangledName &name = demangler.demangleFunctionName(&method.getFunction());
  CPGProtoNode *methodNode = builder.methodNode();
  (*methodNode) //
      .setFullName(name.fullName)
      .setASTParentType("NAMESPACE_BLOCK")
      .setASTParentFullName(file.getGlobalNamespaceName())
      .setIsExternal(method.isExternal())
      .setSignature(getTypeName(method.getFunction().getFunctionType()))
      .setOrder(0);

  llvm::Function *fun = &(method.getFunction());
  llvm::DISubprogram *sub = fun->getSubprogram();
  if (sub != nullptr) {
    methodNode->setLineNumber(sub->getLine());
  }
  if (sub != nullptr && !sub->getName().empty()) {
    methodNode->setName(sub->getName());
  } else {
    methodNode->setName(name.name);
  }
  return methodNode;
}

CPGProtoNode *CPGEmitter::emitMethodReturnNode(const CPGMethod &method) {
  std::string methodReturnType = getTypeName(method.getReturnType());
  CPGProtoNode *methodReturnNode = builder.methodReturnNode();
  (*methodReturnNode) //
      .setOrder(0)
      .setTypeFullName(methodReturnType)
      .setCode(methodReturnType)
      .setEvaluationStrategy("BY_VALUE");
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
  if (auto inst = llvm::dyn_cast<llvm::Instruction>(value)) {
    if (inst->getMetadata(inlineMD) != nullptr) {
      if (auto gep = llvm::dyn_cast<llvm::GetElementPtrInst>(value)) {
        return emitGEP(gep);
      } else if (auto cast = llvm::dyn_cast<llvm::CastInst>(value)) {
        return emitCast(cast);
      } else if (auto load = llvm::dyn_cast<llvm::LoadInst>(value)) {
        return emitDereference(load);
      }
    }
  }

  if (isGlobal(value)) {
    auto global = llvm::dyn_cast<llvm::GlobalVariable>(value);
    if (global && global->hasInitializer()) {
      return emitAddressOf(emitRefOrConstant(global->getInitializer()),
                           getTypeName(global->getType()));
    }
  }

  if (isLocal(value) || isGlobal(value)) {
    CPGProtoNode *valueRef = builder.identifierNode();
    (*valueRef) //
        .setName(value->getName())
        .setCode(value->getName())
        .setTypeFullName(getTypeName(value->getType()));

    if (!isGlobal(value)) {
      builder.connectREF(valueRef, getLocal(value));
    }

    resolveConnections(valueRef, {});
    setLineInfo(valueRef);
    return valueRef;
  }
  if (llvm::isa<llvm::ConstantExpr>(value)) {
    return emitConstantExpr(llvm::cast<llvm::ConstantExpr>(value));
  }

  return emitConstant(value);
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

  if (auto constantExpr = llvm::dyn_cast<llvm::ConstantExpr>(value)) {
    return emitConstantExpr(constantExpr);
  } else if (auto function = llvm::dyn_cast<llvm::Function>(value)) {
    // TODO: handle functions without a name
    DemangledName name = demangler.demangleFunctionName(function);
    CPGProtoNode *methodRef = builder.methodRef();
    (*methodRef) //
        .setCode(name.fullName)
        .setMethodInstFullName(name.fullName)
        .setMethodFullName(name.fullName);
    resolveConnections(methodRef, {});
    setLineInfo(methodRef);
    return methodRef;
  }

  CPGProtoNode *literalNode = builder.literalNode();
  literalNode->setTypeFullName(getTypeName(value->getType()));
  resolveConnections(literalNode, {});
  setLineInfo(literalNode);

  switch (value->getValueID()) {
  case llvm::Value::ValueTy::ConstantIntVal:
    literalNode->setCode(constantIntToString(llvm::dyn_cast<llvm::ConstantInt>(value)->getValue()));
    break;
  case llvm::Value::ValueTy::ConstantPointerNullVal:
    literalNode->setCode("nullptr");
    break;
  case llvm::Value::ValueTy::ConstantFPVal: {
    llvm::SmallVector<char, 16> tmp;
    llvm::dyn_cast<llvm::ConstantFP>(value)->getValueAPF().toString(tmp);
    literalNode->setCode(std::string(tmp.begin(), tmp.end()));
    break;
  }
  case llvm::Value::ValueTy::ConstantAggregateZeroVal:
    literalNode->setCode("zero initialized");
    break;
  case llvm::Value::ValueTy::UndefValueVal:
    literalNode->setCode("undef");
    break;
  case llvm::Value::ValueTy::MetadataAsValueVal:
    literalNode->setCode(std::string("metadata ") + valueToString(value));
    break;
  case llvm::Value::ValueTy::InlineAsmVal:
    literalNode->setCode(llvm::dyn_cast<llvm::InlineAsm>(value)->getAsmString());
    break;

  case llvm::Value::ValueTy::ConstantDataArrayVal: {
    auto cda = llvm::dyn_cast<llvm::ConstantDataArray>(value);

    if (cda->isCString()) {
      llvm::StringRef str = cda->getAsCString();
      if (llvm::json::isUTF8(str, nullptr)) {
        literalNode->setCode(str);
      } else {
        std::stringstream stream;
        for (unsigned i = 0; i < cda->getNumElements() - 1; i++) {
          stream << "0x" << std::setw(2) << std::hex << std::setfill('0')
                 << cda->getElementAsInteger(i);
        }
        literalNode->setCode(stream.str());
      }
      break;
    } // else fall through
  }
  case llvm::Value::ValueTy::ConstantStructVal:
  case llvm::Value::ValueTy::BlockAddressVal:
  case llvm::Value::ValueTy::ConstantVectorVal:
  case llvm::Value::ValueTy::ConstantArrayVal:
  case llvm::Value::ValueTy::ConstantDataVectorVal:
    literalNode->setCode(valueToString(value));
    break;

  case llvm::Value::ValueTy::GlobalVariableVal:
  case llvm::Value::ValueTy::GlobalAliasVal:

  case llvm::Value::ValueTy::ConstantExprVal:
  case llvm::Value::ValueTy::FunctionVal:
  case llvm::Value::ValueTy::GlobalIFuncVal:
    // unreachable: We already did these in front.
    logger.logWarning(std::string("Unreachable reached when processing constant."));

  default:
    logger.logWarning(std::string("Cannot handle constant: ") + valueToString(value) +
                      std::string(" of ValueID ") + std::to_string(value->getValueID()) + "\n");
    literalNode->setCode(valueToString(value));
  }

  return literalNode;
}

CPGProtoNode *CPGEmitter::emitConstant(unsigned int c) {
  CPGProtoNode *literalNode = builder.literalNode();
  (*literalNode) //
      .setTypeFullName("i32")
      .setCode(std::to_string(c));
  resolveConnections(literalNode, {});
  setLineInfo(literalNode);
  return literalNode;
}

CPGProtoNode *CPGEmitter::emitConstantExpr(llvm::ConstantExpr *constantExpr) {
  llvm::Instruction *constInstruction = constantExpr->getAsInstruction();
  ValGuard guard(constInstruction);
  if (auto gep = llvm::dyn_cast<llvm::GetElementPtrInst>(constInstruction)) {
    return emitGEP(gep);
  } else if (auto cast = llvm::dyn_cast<llvm::CastInst>(constInstruction)) {
    return emitCast(cast);
  } else if (auto binary = llvm::dyn_cast<llvm::BinaryOperator>(constInstruction)) {
    return emitBinaryCall(binary);
  } else if (auto cmp = llvm::dyn_cast<llvm::CmpInst>(constInstruction)) {
    return emitCmpCall(cmp);
  } else if (auto select = llvm::dyn_cast<llvm::SelectInst>(constInstruction)) {
    return emitSelect(select);
  } else if (auto extractValue = llvm::dyn_cast<llvm::ExtractValueInst>(constInstruction)) {
    return emitExtractValue(extractValue);
  } else if (auto insertValue = llvm::dyn_cast<llvm::InsertValueInst>(constInstruction)) {
    return emitInsertValue(insertValue);
  } else if (auto extractElement = llvm::dyn_cast<llvm::ExtractElementInst>(constInstruction)) {
    return emitExtractElement(extractElement);
  } else if (auto insertElement = llvm::dyn_cast<llvm::InsertElementInst>(constInstruction)) {
    return emitInsertElement(insertElement);
  } else if (auto shuffleVector = llvm::dyn_cast<llvm::ShuffleVectorInst>(constInstruction)) {
    return emitShuffleVector(shuffleVector);
  } else if (auto unary = llvm::dyn_cast<llvm::UnaryOperator>(constInstruction)) {
    return emitUnaryOperator(unary);
  } else {
    logger.logWarning(std::string("Cannot handle constant expression yet: ") +
                      valueToString(constInstruction) + std::string(" of ValueID ") +
                      std::to_string(constInstruction->getValueID()) + "\n");

    CPGProtoNode *unhandled = builder.unknownNode();
    unhandled->setCode(valueToString(constInstruction));
    resolveConnections(unhandled, {});
    setLineInfo(unhandled);

    return unhandled;
  }
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
      .setEvaluationStrategy("BY_VALUE")
      .setOrder(order);
  return parameterInNode;
}

CPGProtoNode *CPGEmitter::emitAssignCall(const llvm::Type *type, CPGProtoNode *lhs,
                                         CPGProtoNode *rhs) {
  return resolveConnections(
      emitGenericOp(
          "<operator>.assignment", "<operator>.assignment", getTypeName(type), "ANY (ANY)"),
      { lhs, rhs });
}

CPGProtoNode *CPGEmitter::emitAllocaCall(const llvm::Value *value) {
  return resolveConnections(
      emitGenericOp("<operator>.alloca", "alloca", getTypeName(value->getType()), "ANY ()"), {});
}

CPGProtoNode *CPGEmitter::emitIndirectionCall(const llvm::Type *type, CPGProtoNode *pointerRef) {
  return resolveConnections(
      emitGenericOp("<operator>.indirection", "store", getTypeName(type), "ANY (ANY)"),
      { pointerRef });
}

CPGProtoNode *CPGEmitter::emitDereference(llvm::LoadInst *load) {
  CPGProtoNode *dereference =
      emitGenericOp("<operator>.indirection", "load", getTypeName(load->getType()), "ANY (ANY)");
  /// Special-casing here to handle the class method call resolution in ObjC
  if (load->hasMetadata()) {
    if (llvm::MDNode *md = load->getMetadata("shiftleft.objc_type_hint")) {
      assert(md->getNumOperands() == 1);
      auto *typeHint = llvm::cast<llvm::MDString>(md->getOperand(0).get());
      dereference->setDynamicTypeHintFullName(typeHint->getString());
    }
  }
  return resolveConnections(dereference, { emitRefOrConstant(load->getPointerOperand()) });
}

CPGProtoNode *CPGEmitter::emitBinaryCall(const llvm::BinaryOperator *binary) {
  std::string name(binaryOperatorName(binary));
  return resolveConnections(
      emitGenericOp(name, name, getTypeName(binary->getType()), "ANY (ANY, ANY)"),
      { emitRefOrConstant(binary->getOperand(0)), emitRefOrConstant(binary->getOperand(1)) });
}

CPGProtoNode *CPGEmitter::emitCmpCall(const llvm::CmpInst *comparison) {
  std::string name(comparisonOperatorName(comparison));
  // return type is i1 or <? x i1>. Be safe or emit i1?
  return resolveConnections(
      emitGenericOp(name, name, getTypeName(comparison->getType()), "ANY (ANY, ANY)"),
      { emitRefOrConstant(comparison->getOperand(0)),
        emitRefOrConstant(comparison->getOperand(1)) });
}

CPGProtoNode *CPGEmitter::emitCast(const llvm::CastInst *instruction) {
  std::string name(castOperatorName(instruction));
  return resolveConnections(
      emitGenericOp(name, name, getTypeName(instruction->getDestTy()), "ANY (ANY)"),
      { emitRefOrConstant(instruction->getOperand(0)) });
}

CPGProtoNode *CPGEmitter::emitSelect(llvm::SelectInst *instruction) {
  // first arg type is i1 or <? x i1>. Be safe or emit i1?
  return resolveConnections(emitGenericOp("<operator>.select",
                                          "<operator>.select",
                                          getTypeName(instruction->getType()),
                                          "ANY (ANY, ANY, ANY)"),
                            { emitRefOrConstant(instruction->getCondition()),
                              emitRefOrConstant(instruction->getTrueValue()),
                              emitRefOrConstant(instruction->getFalseValue()) });
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
  //  Each access returns a pointer.
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

  /// If the GEP only has zero indices, then we emit the operand itself bypassing the GEP completely

  llvm::Value *element = instruction->getOperand(0);
  llvm::Value *index = instruction->getOperand(1);
  std::vector<llvm::Value *> indexes = { index };

  llvm::Type *baseType = instruction->getSourceElementType();
  llvm::Type *indexType = llvm::GetElementPtrInst::getIndexedType(baseType, indexes);
  CPGProtoNode *access = nullptr;

  size_t numOperands = instruction->getNumOperands();
  if (numOperands == 2) {
    indexType = indexType->getPointerTo();
  }

  access = emitGenericOp(
      "<operator>.pointerShift", "getelementptr", getTypeName(indexType), "ANY (ANY)");
  resolveConnections(access, { emitRefOrConstant(element), emitRefOrConstant(index) });

  for (size_t i = 2; i < numOperands; i++) {
    auto *structAccessType = llvm::dyn_cast<llvm::StructType>(indexType);
    bool isMemberAccess = structAccessType != nullptr && structAccessType->hasName();

    index = instruction->getOperand(i);
    indexes.push_back(index);
    indexType = llvm::GetElementPtrInst::getIndexedType(baseType, indexes);

    CPGProtoNode *lhs = access, *rhs = nullptr;
    if (isMemberAccess) {
      rhs = builder.fieldIdentifierNode();
      rhs->setEntry(rhs->getID());

      auto constIndex = llvm::cast<llvm::ConstantInt>(index);
      uint64_t indexValue = *constIndex->getValue().getRawData();
      std::vector<std::string> &structMembers = typeEmitter.getStructMembers(structAccessType);
      std::string indexName;
      if (indexValue < structMembers.size()) {
        indexName = structMembers[indexValue];
      } else {
        indexName = constantIntToString(constIndex->getValue());
      }

      (*rhs) //
          .setCanonicalName(indexName)
          .setCode(indexName);
    } else {
      rhs = emitRefOrConstant(index);
    }

    // the proper return type when nesting is the return type of the GEP, i.e. the pointer type
    // fixme once this doesn't collide with type info anymore
    access = emitGEPAccess(
        i == numOperands - 1 ? indexType->getPointerTo() : indexType, index, isMemberAccess);
    resolveConnections(access, { lhs, rhs });
  }

  return access;
}

CPGProtoNode *CPGEmitter::emitInsertValue(llvm::InsertValueInst *instruction) {
  /*
    We have an issue: We would need an extra temp to get dataflow semantics correctly.
    So we just push out the function as it appears in llvm?
  */
  std::vector<CPGProtoNode *> children;
  children.push_back(emitRefOrConstant(instruction->getAggregateOperand()));
  children.push_back(emitRefOrConstant(instruction->getInsertedValueOperand()));
  auto indices = instruction->getIndices();
  for (unsigned int i = 0; i < instruction->getNumIndices(); i++) {
    children.push_back(emitConstant(indices[i]));
  }
  return resolveConnections(emitGenericOp("<operator>.insertValue",
                                          "insertvalue",
                                          getTypeName(instruction->getType()),
                                          "ANY (ANY)"),
                            children);
}

CPGProtoNode *CPGEmitter::emitInsertElement(llvm::InsertElementInst *instruction) {
  /*
    We have an issue: We would need an extra temp to get dataflow semantics correctly.
    So we just push out the function as it appears in llvm?
  */
  return resolveConnections(emitGenericOp("<operator>.insertElement",
                                          "insertelement",
                                          getTypeName(instruction->getType()),
                                          "ANY (ANY ANY)"),
                            { emitRefOrConstant(instruction->getOperand(0)),
                              emitRefOrConstant(instruction->getOperand(1)),
                              emitRefOrConstant(instruction->getOperand(2)) });
}

CPGProtoNode *CPGEmitter::emitExtractValue(llvm::ExtractValueInst *instruction) {
  llvm::Value *from = instruction->getAggregateOperand();
  llvm::Type *indexType = from->getType();
  CPGProtoNode *agg = emitRefOrConstant(from);
  auto indices = instruction->getIndices();

  for (unsigned int i = 0; i < instruction->getNumIndices(); i++) {
    unsigned int index = indices[i];
    CPGProtoNode *idxCpg = emitConstant(index);
    bool isStruct = indexType->isStructTy();
    indexType = nextIndexType(indexType, index);
    CPGProtoNode *access = emitExtract(indexType, index, isStruct);
    resolveConnections(access, { agg, idxCpg });
    agg = access;
  }
  return agg;
}

CPGProtoNode *CPGEmitter::emitGEPAccess(const llvm::Type *type, llvm::Value *index,
                                        bool memberAccess) {
  std::string name("<operator>.indexAccess"); // todo: This is not entirely correct
  if (memberAccess) {
    name = "<operator>.getElementPtr";
  }
  return resolveConnections(emitGenericOp(name, "getelementptr", getTypeName(type), "ANY (ANY)"),
                            {});
}

CPGProtoNode *CPGEmitter::emitExtract(const llvm::Type *type, unsigned int idx, bool memberAccess) {
  // This is problematic for the dataflow tracker. TODO revisit
  std::string name("<operator>.indexAccess");
  /* reconsider later:
  if (memberAccess) {
    name = "<operator>.memberAccess";
  }
  */
  return resolveConnections(emitGenericOp(name, "extractvalue", getTypeName(type), "ANY (ANY)"),
                            {});
}

CPGProtoNode *CPGEmitter::emitUnaryOperator(const llvm::UnaryOperator *instruction) {
  std::string name(unaryOperatorName(instruction));
  return resolveConnections(
      emitGenericOp(name, name, getTypeName(instruction->getType()), "ANY (ANY)"),
      { emitRefOrConstant(instruction->getOperand(0)) });
}

CPGProtoNode *CPGEmitter::emitFunctionCall(llvm::CallBase *instruction) {
  if (!instruction->getCalledFunction()) {
    return emitIndirectFunctionCall(instruction);
  }
  return emitDirectFunctionCall(instruction);
}

static bool isObjCCall(llvm::CallBase *instruction) {
  llvm::Function *msgSend = instruction->getModule()->getFunction("objc_msgSend");
  if (!msgSend) {
    return false;
  }
  llvm::Value *operand = instruction->getCalledOperand();
  if (auto constExpr = llvm::dyn_cast<llvm::ConstantExpr>(operand)) {
    llvm::Instruction *constInst = constExpr->getAsInstruction();
    ValGuard guard(constInst);
    if (auto cast = llvm::dyn_cast<llvm::BitCastInst>(constInst)) {
      if (auto function = llvm::dyn_cast<llvm::Function>(cast->getOperand(0))) {
        if (function == msgSend) {
          return true;
        }
      }
    }
  }

  return false;
}

llvm::Function *getCastFunction(llvm::CallBase *instruction) {
  if (auto constantExpr = llvm::dyn_cast<llvm::ConstantExpr>(instruction->getCalledOperand())) {
    if (constantExpr->getNumOperands() == 0) {
      return nullptr;
    }
    return llvm::dyn_cast<llvm::Function>(constantExpr->getOperand(0));
  }
  return nullptr;
}

static bool isCastCall(llvm::CallBase *instruction) {
  return getCastFunction(instruction) != nullptr;
}

CPGProtoNode *CPGEmitter::emitIndirectFunctionCall(llvm::CallBase *instruction) {
  if (isObjCCall(instruction)) {
    return emitObjCFunctionCall(instruction);
  }
  if (isCastCall(instruction)) {
    return emitCastFunctionCall(instruction);
  }

  std::string name("indirect_call");
  std::string dispatch("DYNAMIC_DISPATCH");
  std::string code(name);
  /// TODO: Find a better way to represent inline assembly
  if (instruction->isInlineAsm()) {
    name = "inline_asm";
    dispatch = "STATIC_DISPATCH";
    code = valueToString(instruction->getCalledOperand());
  }
  CPGProtoNode *callNode = builder.functionCallNode();
  (*callNode) //
      .setName(name)
      .setCode(code)
      .setTypeFullName(getTypeName(instruction->getType()))
      .setMethodInstFullName(name)
      .setMethodFullName(name)
      .setSignature(getTypeName(instruction->getCalledOperand()->getType()))
      .setDispatchType(dispatch);

  setLineInfo(callNode);
  CPGProtoNode *receiver = emitRefOrConstant(instruction->getCalledOperand());
  builder.connectReceiver(callNode, receiver);

  std::vector<CPGProtoNode *> children({ receiver });
  for (const llvm::Use &argument : instruction->args()) {
    CPGProtoNode *arg = emitRefOrConstant(argument.get());
    children.push_back(arg);
  }
  resolveConnections(callNode, children);
  return callNode;
}

CPGProtoNode *CPGEmitter::emitDirectFunctionCall(llvm::CallBase *instruction) {
  assert(instruction->getCalledFunction());
  CPGProtoNode *call = builder.functionCallNode();
  DemangledName demangledName = demangler.demangleFunctionName(instruction->getCalledFunction());
  (*call) //
      .setName(demangledName.name)
      .setCode(demangledName.name)
      .setTypeFullName(getTypeName(instruction->getType()))
      .setMethodFullName(demangledName.fullName)
      .setSignature(getTypeName(instruction->getCalledFunction()->getFunctionType()))
      .setDispatchType("STATIC_DISPATCH");

  setLineInfo(call);
  std::vector<CPGProtoNode *> children;
  for (const llvm::Use &argument : instruction->args()) {
    CPGProtoNode *arg = emitRefOrConstant(argument.get());
    children.push_back(arg);
  }

  resolveConnections(call, children);
  return call;
}

static llvm::Constant *getConstInitializer(llvm::Value *globalVariable) {
  auto global = llvm::dyn_cast<llvm::GlobalVariable>(globalVariable);
  if (global && global->hasInitializer()) {
    if (auto constant = llvm::dyn_cast<llvm::Constant>(global->getInitializer())) {
      return constant;
    }
  }

  return nullptr;
}

static std::string objcSelectorName(llvm::Value *selector) {
  std::string defaultName("objc_msgSend");
  if (auto load = llvm::dyn_cast<llvm::LoadInst>(selector)) {
    llvm::Constant *constReference = getConstInitializer(load->getPointerOperand());
    if (!constReference) {
      return defaultName;
    }
    if (auto constReferenceExpr = llvm::dyn_cast<llvm::ConstantExpr>(constReference)) {
      llvm::Instruction *referenceInst = constReferenceExpr->getAsInstruction();
      ValGuard referenceGuard(referenceInst);
      if (auto gep = llvm::dyn_cast<llvm::GetElementPtrInst>(referenceInst)) {
        llvm::Constant *constSelector = getConstInitializer(gep->getPointerOperand());
        if (!constSelector) {
          return defaultName;
        }
        if (auto constSelectorString = llvm::dyn_cast<llvm::ConstantDataArray>(constSelector)) {
          if (constSelectorString->isCString()) {
            return constSelectorString->getAsCString();
          }
        }
      }
    }
  }

  return defaultName;
}

CPGProtoNode *CPGEmitter::emitObjCFunctionCall(llvm::CallBase *instruction) {
  llvm::Value *receiver = instruction->getArgOperand(0);
  if (auto cast = llvm::dyn_cast<llvm::BitCastInst>(receiver)) {
    receiver = cast->getOperand(0);
    if (cast->getNumUses() != 1) {
      std::string message;
      llvm::raw_string_ostream stream(message);
      stream << "objc_msgSend receiver has more than one use:\n";
      for (auto &use : cast->uses()) {
        use->printAsOperand(stream);
        stream << "\n";
      }
      logger.logWarning(stream.str());
    }
  } else {
    std::string message;
    llvm::raw_string_ostream stream(message);
    stream << "objc_msgSend receiver is not a bitcast: ";
    receiver->printAsOperand(stream);
    logger.logWarning(stream.str());
  }

  llvm::Value *selector = instruction->getArgOperand(1);

  CPGProtoNode *callNode = builder.functionCallNode();
  std::string name = objcSelectorName(selector);
  (*callNode) //
      .setName(name)
      .setCode(name)
      .setTypeFullName(getTypeName(receiver->getType()))
      .setSignature("")
      .setDispatchType("DYNAMIC_DISPATCH");

  setLineInfo(callNode);
  CPGProtoNode *receiverNode = emitRefOrConstant(receiver);
  builder.connectReceiver(callNode, receiverNode);

  std::vector<CPGProtoNode *> children({ receiverNode });

  for (unsigned i = 1; i < instruction->arg_size(); i++) {
    llvm::Value *argument = instruction->getArgOperand(i);
    CPGProtoNode *arg = emitRefOrConstant(argument);
    children.push_back(arg);
  }

  resolveConnections(callNode, children);
  return callNode;
}

CPGProtoNode *CPGEmitter::emitCastFunctionCall(llvm::CallBase *instruction) {
  llvm::Function *function = getCastFunction(instruction);
  assert(function);

  CPGProtoNode *callNode = builder.functionCallNode();
  std::string name = function->getName();
  (*callNode) //
      .setName(name)
      .setCode(name)
      .setTypeFullName(getTypeName(instruction->getType()))
      .setMethodInstFullName(name)
      .setMethodFullName(name)
      .setSignature(getTypeName(function->getFunctionType()))
      .setDispatchType("STATIC_DISPATCH");

  setLineInfo(callNode);

  std::vector<CPGProtoNode *> children;
  for (const llvm::Use &argument : instruction->args()) {
    CPGProtoNode *arg = emitRefOrConstant(argument.get());
    children.push_back(arg);
  }
  resolveConnections(callNode, children);
  return callNode;
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

CPGProtoNode *CPGEmitter::emitGenericOp(const std::string &fullname, const std::string &code,
                                        const std::string &returnType,
                                        const std::string &signature) {

  CPGProtoNode *call = builder.functionCallNode();
  (*call) //
      .setName(fullname)
      .setCode(code)
      .setTypeFullName(returnType)
      .setMethodInstFullName(fullname)
      .setMethodFullName(fullname)
      .setSignature(signature)
      .setDispatchType("STATIC_DISPATCH");

  setLineInfo(call);
  call->setEntry(call->getID());
  return call;
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

CPGProtoNode *CPGEmitter::resolveConnections(CPGProtoNode *parent,
                                             std::vector<CPGProtoNode *> children) {
  resolveCFGConnections(parent, children);
  resolveASTConnections(parent, children);
  resolveArgumentConnections(parent, children);
  return parent;
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

void CPGEmitter::resolveArgumentConnections(CPGProtoNode *call,
                                            std::vector<CPGProtoNode *> arguments) {
  for (CPGProtoNode *argument : arguments) {
    builder.connectArgument(call, argument);
  }
}

void CPGEmitter::updateLineInfo(const llvm::Instruction *inst) {
  if (inst != nullptr) {
    const llvm::DebugLoc *DILoc = &(inst->getDebugLoc());
    if (DILoc && DILoc->get() && DILoc->getInlinedAt() == nullptr) {
      lineNumber = DILoc->getLine();
      columnNumber = DILoc->getCol();
      return;
    }
  }
  lineNumber = 0;
  columnNumber = 0;
}

void CPGEmitter::setLineInfo(CPGProtoNode *node) {
  if (lineNumber && node) {
    node->setLineNumber(lineNumber);
  }
  if (columnNumber && node) {
    node->setColumnNumber(columnNumber);
  }
}

llvm2cpg::CPGProtoNode *CPGEmitter::emitAddressOf(llvm2cpg::CPGProtoNode *node,
                                                  const std::string &returnTyp) {
  return resolveConnections(emitGenericOp("<operator>.addressOf", "addressOf", returnTyp, "ANY"),
                            { node });
}

std::string CPGEmitter::getTypeName(const llvm::Type *type) {
  return typeEmitter.recordType(type, file.getGlobalNamespaceName());
}
