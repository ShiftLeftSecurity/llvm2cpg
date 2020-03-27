#include "llvm2cpg/Transforms/Transforms.h"
#include "llvm2cpg/Logger/CPGLogger.h"
#include "llvm2cpg/Transforms/CustomPasses.h"

#include <llvm/IR/DebugInfo.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Pass.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/IPO/FunctionAttrs.h>
#include <llvm/Transforms/IPO/InferFunctionAttrs.h>
#include <llvm/Transforms/Scalar/DCE.h>
#include <llvm/Transforms/Scalar/SimplifyCFG.h>
#include <llvm/Transforms/Utils/Local.h>
#include <llvm/Transforms/Utils/LowerInvoke.h>
#include <llvm2cpg/Traversals/ObjCTraversal.h>
#include <unordered_map>
#include <unordered_set>

using namespace llvm2cpg;

Transforms::Transforms(CPGLogger &log, bool inlineAP, bool simplify, bool inlineStrings)
    : logger(log), inlineAP(inlineAP), simplify(simplify), inlineStrings(inlineStrings) {}

void Transforms::transformBitcode(llvm::Module &bitcode) {
  renameOpaqueObjCTypes(bitcode);
  markObjCTypeHints(bitcode);
  inlineGlobalStrings(bitcode);
  for (llvm::Function &function : bitcode) {
    if (function.isDeclaration()) {
      continue;
    }
  }
  runPasses(bitcode);

  logger.doNothing();
}

void Transforms::markObjCTypeHints(llvm::Module &bitcode) {
  ObjCTraversal traversal(&bitcode);
  for (llvm::GlobalObject &global : bitcode.global_objects()) {
    if (!global.hasName()) {
      continue;
    }

    if (global.getName().startswith("OBJC_CLASSLIST_REFERENCES_$")) {
      auto &classReference = llvm::cast<llvm::GlobalVariable>(global);
      if (!classReference.hasInitializer()) {
        std::string str;
        llvm::raw_string_ostream stream(str);
        stream << "ObjC class reference does not have initializer: ";
        classReference.printAsOperand(stream);
        logger.logWarning(stream.str());
        continue;
      }
      auto *globalVariable = llvm::dyn_cast<llvm::GlobalVariable>(classReference.getInitializer());
      if (!globalVariable) {
        std::string str;
        llvm::raw_string_ostream stream(str);
        stream << "ObjC class reference does not reference global variable: ";
        classReference.getInitializer()->printAsOperand(stream);
        logger.logWarning(stream.str());
        continue;
      }
      ObjCClassDefinition *classDefinition = traversal.objcClassFromGlobalObject(globalVariable);
      if (!classDefinition) {
        logger.logWarning("Cannot initialize ObjC class");
        continue;
      }

      assert(!classDefinition->isMetaclass());

      llvm::LLVMContext &context = global.getContext();
      std::string mdName("shiftleft.objc_type_hint");
      unsigned rootClassMD = context.getMDKindID(mdName);
      llvm::MDNode *paylod = llvm::MDNode::get(
          context, { llvm::MDString::get(context, classDefinition->getName() + "$") });
      for (llvm::Value *user : global.users()) {
        if (auto instruction = llvm::dyn_cast<llvm::Instruction>(user)) {
          instruction->setMetadata(rootClassMD, paylod);
        }
      }
    }
  }
}

void Transforms::renameOpaqueObjCTypes(llvm::Module &bitcode) {
  ObjCTraversal traversal(&bitcode);
  std::vector<ObjCClassDefinition *> worklist = traversal.objcClasses();

  for (ObjCClassDefinition *objcClass : worklist) {
    std::string className = objcClass->getName();
    for (auto &method : traversal.objcClassMethods(objcClass)) {
      llvm::FunctionType *type = method.function->getFunctionType();
      assert(type->getNumParams() >= 2 &&
             "ObjC method expected to have implicit parameters (self and _cmd)");

      if (!llvm::isa<llvm::PointerType>(type->getParamType(0))) {
        continue;
      }

      auto *selfType = llvm::cast<llvm::PointerType>(type->getParamType(0));
      if (!llvm::isa<llvm::StructType>(selfType->getPointerElementType())) {
        continue;
      }

      auto *selfStruct = llvm::cast<llvm::StructType>(selfType->getPointerElementType());
      if (!selfStruct->isOpaque()) {
        continue;
      }
      assert(selfStruct->isOpaque() && "ObjC class types expected to be opaque");
      selfStruct->setName(className);
    }
  }
}

void Transforms::runPasses(llvm::Module &bitcode) {

  llvm::PassBuilder passBuilder;
  bool DebugPM = false;
  llvm::LoopAnalysisManager loopAnalysisManager(DebugPM);
  llvm::FunctionAnalysisManager functionAnalysisManager(DebugPM);
  llvm::CGSCCAnalysisManager ipoAnalysisManager(DebugPM);
  llvm::ModuleAnalysisManager moduleAnalysisManager(DebugPM);
  {
    // TODO: use custom pipeline
    // may want to use objC-aware passes (named *ARC*)
    llvm::AAManager AAManager = passBuilder.buildDefaultAAPipeline();
    functionAnalysisManager.registerPass([&] { return std::move(AAManager); });
  }
  passBuilder.registerModuleAnalyses(moduleAnalysisManager);
  passBuilder.registerCGSCCAnalyses(ipoAnalysisManager);
  passBuilder.registerFunctionAnalyses(functionAnalysisManager);
  passBuilder.registerLoopAnalyses(loopAnalysisManager);
  passBuilder.crossRegisterProxies(
      loopAnalysisManager, functionAnalysisManager, ipoAnalysisManager, moduleAnalysisManager);
  // end of boilerplate

  llvm::ModulePassManager modulePassManager(DebugPM);

  modulePassManager.addPass(llvm::VerifierPass());
  modulePassManager.addPass(customPasses::StripOptNonePass());
  {
    llvm::FunctionPassManager FPM(DebugPM);
    FPM.addPass(llvm::LowerInvokePass());
    if (simplify) {
      FPM.addPass(llvm::DCEPass());
    }
    modulePassManager.addPass(llvm::createModuleToFunctionPassAdaptor(std::move(FPM)));
  }

  // More or less cribbed from standard pipeline. The passes are supposed to run in exactly this
  // order. optionally could run multiple times.
  modulePassManager.addPass(llvm::InferFunctionAttrsPass());
  {
    llvm::CGSCCPassManager IPOPM(DebugPM);
    IPOPM.addPass(llvm::PostOrderFunctionAttrsPass());
    modulePassManager.addPass(llvm::createModuleToPostOrderCGSCCPassAdaptor(std::move(IPOPM)));
  }
  modulePassManager.addPass(llvm::ReversePostOrderFunctionAttrsPass());
  {
    llvm::FunctionPassManager FPM(DebugPM);
    FPM.addPass(customPasses::DemotePhiPass());
    if (simplify) {
      FPM.addPass(
          llvm::SimplifyCFGPass(llvm::SimplifyCFGOptions(1, false, false, false, false, nullptr)));
      FPM.addPass(llvm::DCEPass());
      // SimplifyCFGPass could maybe re-introduce phis
      FPM.addPass(customPasses::DemotePhiPass());
    }

    if (inlineAP) {
      FPM.addPass(customPasses::LoadInlinePass());
    }
    modulePassManager.addPass(llvm::createModuleToFunctionPassAdaptor(std::move(FPM)));
  }
  modulePassManager.addPass(llvm::VerifierPass());
  modulePassManager.run(bitcode, moduleAnalysisManager);
}

static std::unordered_set<llvm::Type *> findObjCStringTagTypes(llvm::Module &bitcode) {
  std::unordered_set<llvm::Type *> types;
  for (llvm::StructType *type : bitcode.getIdentifiedStructTypes()) {
    if (type->hasName() && !type->isOpaque() &&
        type->getName().startswith("struct.__NSConstantString_tag")) {
      types.insert(type);
    }
  }
  return types;
}

static bool isInlineableConstGEP(llvm::Value *value) {
  if (!value) {
    return false;
  }
  auto constExpr = llvm::dyn_cast<llvm::ConstantExpr>(value);
  if (constExpr && constExpr->getOpcode() == llvm::Instruction::GetElementPtr) {
    // We expect to see a const getelementptr @str, 0, 0
    llvm::Constant *index_0 = constExpr->getOperand(1);
    llvm::Constant *index_1 = constExpr->getOperand(2);
    if (index_0->isNullValue() && index_1->isNullValue()) {
      return true;
    }
  }
  return false;
}

static bool isInlineableConstBitcast(llvm::Value *value) {
  if (value) {
    auto constExpr = llvm::dyn_cast<llvm::ConstantExpr>(value);
    if (constExpr && constExpr->getOpcode() == llvm::Instruction::BitCast) {
      return true;
    }
  }
  return false;
}

static bool isInlineableStringUserInstruction(llvm::Value *value) {
  if (value) {
    return llvm::isa<llvm::LoadInst>(value) || llvm::isa<llvm::StoreInst>(value) ||
           llvm::isa<llvm::CallBase>(value);
  }
  return false;
}

/// Extracts the pointer operand from the const GEP expression
/// Extracts the pointer only if there are two 0 indices
static llvm::Value *getGEPPointer(llvm::Value *value) {
  if (isInlineableConstGEP(value)) {
    return llvm::cast<llvm::ConstantExpr>(value)->getOperand(0);
  }
  return nullptr;
}

void Transforms::inlineGlobalStrings(llvm::Module &bitcode) {
  if (!inlineStrings) {
    return;
  }
  // clang-format off
  /// There are three cases in which we want to inline strings:
  ///
  /// 1. A global variable initialized as a string:
  ///   @.str.1 = private unnamed_addr constant [6 x i8] c"Hello\00"
  ///
  /// 2. ObjC string wrapped into a __NSConstantString_tag struct:
  ///   @.str.1 = private unnamed_addr constant [6 x i8] c"Hello\00"
  ///   @_unnamed_cfstring_ = private global %struct.__NSConstantString_tag {
  ///     i32* getelementptr inbounds ([0 x i32], [0 x i32]* @__CFConstantStringClassReference, i32 0, i32 0),
  ///     i32 1992,
  ///     i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str.1, i32 0, i32 0),
  ///     i64 5
  ///   }
  ///
  /// 3. ObjC selectors referenced indirectly:
  ///   @OBJC_METH_VAR_NAME_ = private unnamed_addr constant [13 x i8] c"createObject\00"
  ///   @OBJC_SELECTOR_REFERENCES_ = private externally_initialized global i8* getelementptr inbounds ([13 x i8], [13 x i8]* @OBJC_METH_VAR_NAME_, i32 0, i32 0)
  ///
  /// Each inlineable string appears in a ConstantExpr (either within a bitcast or within a getelementptr) or as an operand of Load/Store/Call instruction.
  /// The ConstantExpr's may appear in several contexts, but we inline them only within load, store, and call instructions.
  /// If an inlineable string is not wrapped into a ConstantExpr and appears within a load/store/call instruction, then we also inline it.
  /// Examples:
  /// 1.
  ///   @global = constant [6 x i8] c"Hello\00"
  ///   %x = load bitcast @global
  ///   Not-inlined: <operator>.assignment(x, <operator>.indirection(<operator>.cast(<operator>.addressOf(@global)))
  ///   Inlined: <operator>.assignment(x, "Hello")
  ///
  /// 2.
  ///   @global = constant [6 x i8] c"Hello\00"
  ///   %x = load @global
  ///   Not-inlined: <operator>.assignment(x, <operator>.indirection(<operator>.addressOf(@global))
  ///   Inlined: <operator>.assignment(x, "Hello")
  ///
  /// 3.
  ///   @global = constant [6 x i8] c"Hello\00"
  ///   %x = load getelementptr @global, 0, 0
  ///   Not-inlined: <operator>.assignment(x, <operator>.pointerShift(<operator>.pointerShift(<operator>.addressOf(@global), 0), 0))
  ///   Inlined: <operator>.assignment(x, "Hello")
  ///
  /// 4.
  ///   @.str.1 = private unnamed_addr constant [6 x i8] c"Hello\00"
  ///   @_unnamed_cfstring_ = private global %struct.__NSConstantString_tag {
  ///     i32* getelementptr inbounds ([0 x i32], [0 x i32]* @__CFConstantStringClassReference, i32 0, i32 0),
  ///     i32 1992,
  ///     i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str.1, i32 0, i32 0),
  ///     i64 5
  ///   }
  ///   store %opaque* bitcast (%struct.__NSConstantString_tag* @_unnamed_cfstring_ to %0*), %opaque** %x
  ///   Not-inlined: <operator>.assignment(<operator>.indirection(x), <operator>.cast(<operator>.addressOf(@_unnamed_cfstring_)))
  ///   Inlined: <operator>.assignment(<operator>.indirection(x), "Hello")
  ///
  /// 5.
  ///   @str = private unnamed_addr constant [6 x i8] c"Hello\00"
  ///   call i32 @printf(i8* getelementptr ([6 x i8], [6 x i8]* @str, i32 0, i32 0))
  ///   Not-inlined: printf(<operator>.pointerShift(<operator>.pointerShift(<operator>.addressOf(@str), 0), 0))
  ///   Inlined: printf("Hello")
  ///
  ///  **Notes**:
  ///     - we do not inline GEPs that have more/less than 2 indices
  ///     - we do not inline GEPs which have non-zero indices
  // clang-format on

  std::unordered_set<llvm::Type *> objcTypes = findObjCStringTagTypes(bitcode);
  std::unordered_map<llvm::Value *, llvm::Constant *> inlines;
  std::vector<llvm::GlobalVariable *> inlineableGlobals;

  for (llvm::GlobalVariable &global : bitcode.globals()) {
    if (global.hasInitializer()) {
      if (auto *init = llvm::dyn_cast<llvm::ConstantDataArray>(global.getInitializer())) {
        if (init->isCString()) {
          inlines[&global] = init;
        }
      } else if (!objcTypes.empty() &&
                 objcTypes.count(global.getType()->getPointerElementType()) != 0) {
        inlineableGlobals.push_back(&global);
      } else if (global.hasName() && global.getName().startswith("OBJC_SELECTOR_REFERENCES_")) {
        inlineableGlobals.push_back(&global);
      }
    }
  }

  for (llvm::GlobalVariable *global : inlineableGlobals) {
    assert(global->hasInitializer());
    if (auto init = llvm::dyn_cast<llvm::ConstantStruct>(global->getInitializer())) {
      // the __NSConstantString_tag should have 4 members
      // the third member being the reference to a constant string
      if (init->getNumOperands() == 4) {
        llvm::Value *stringRef = getGEPPointer(init->getOperand(2));
        if (inlines.count(stringRef)) {
          inlines[global] = inlines[stringRef];
        }
      }
    }
    llvm::Value *stringRef = getGEPPointer(global->getInitializer());
    if (inlines.count(stringRef)) {
      inlines[global] = inlines[stringRef];
    }
  }

  std::unordered_map<llvm::Instruction *, std::unordered_map<llvm::Value *, llvm::Constant *>>
      inlineInstructions;
  llvm::LLVMContext &context = bitcode.getContext();
  std::string mdName("shiftleft.inline_string_literal");
  unsigned inlineStringMD = context.getMDKindID(mdName);
  for (auto &pair : inlines) {
    auto *global = llvm::cast<llvm::GlobalVariable>(pair.first);
    llvm::Constant *init = pair.second;
    llvm::MDNode *payload = llvm::MDNode::get(context, { llvm::ConstantAsMetadata::get(init) });
    global->setMetadata(inlineStringMD, payload);

    for (llvm::User *user : global->users()) {
      if (isInlineableConstGEP(user)) {
        for (llvm::User *outerUser : user->users()) {
          if (isInlineableStringUserInstruction(outerUser)) {
            inlineInstructions[llvm::cast<llvm::Instruction>(outerUser)][user] = init;
          }
        }
      } else if (isInlineableConstBitcast(user)) {
        for (llvm::User *outerUser : user->users()) {
          if (isInlineableStringUserInstruction(outerUser)) {
            inlineInstructions[llvm::cast<llvm::Instruction>(outerUser)][user] = init;
          }
        }
      } else if (isInlineableStringUserInstruction(user)) {
        inlineInstructions[llvm::cast<llvm::Instruction>(user)][global] = init;
      }
    }
  }

  llvm::Constant *nullValue = llvm::Constant::getNullValue(llvm::Type::getInt8Ty(context));
  llvm::ConstantAsMetadata *nullMetadata = llvm::ConstantAsMetadata::get(nullValue);
  for (auto &topPair : inlineInstructions) {
    llvm::Instruction *instruction = topPair.first;
    std::vector<llvm::Metadata *> metadata;
    std::unordered_map<llvm::Value *, llvm::Constant *> &initMapping = topPair.second;
    for (llvm::Value *operand : instruction->operands()) {
      if (initMapping.count(operand)) {
        metadata.push_back(llvm::ConstantAsMetadata::get(initMapping[operand]));
      } else {
        metadata.push_back(nullMetadata);
      }
    }
    llvm::MDNode *payload = llvm::MDNode::get(context, metadata);
    instruction->setMetadata(inlineStringMD, payload);
  }
}
