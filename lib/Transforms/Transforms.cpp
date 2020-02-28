#include "llvm2cpg/Transforms/Transforms.h"
#include "llvm2cpg/Transforms/CustomPasses.h"
#include "llvm2cpg/Logger/CPGLogger.h"

#include "llvm/Transforms/Scalar/DCE.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"
#include "llvm/Transforms/Utils/LowerInvoke.h"
#include <llvm/IR/DebugInfo.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/PassManager.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Pass.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/IPO/FunctionAttrs.h>
#include <llvm/Transforms/IPO/InferFunctionAttrs.h>
#include <llvm/Transforms/Utils/Local.h>
#include <llvm2cpg/Traversals/ObjCTraversal.h>

using namespace llvm2cpg;

Transforms::Transforms(CPGLogger &log, bool inlineAP, bool simplify)
    : logger(log), inlineAP(inlineAP), simplify(simplify) {}

void Transforms::transformBitcode(llvm::Module &bitcode) {
  markObjCRootClasses(bitcode);
  renameOpaqueObjCTypes(bitcode);
  markObjCTypeHints(bitcode);
  for (llvm::Function &function : bitcode) {
    if (function.isDeclaration()) {
      continue;
    }
  }
  runPasses(bitcode);

  logger.doNothing();
}


static void markAsRoot(llvm::GlobalVariable &global) {
  llvm::LLVMContext &context = global.getContext();
  std::string mdName("shiftleft.objc_root_class");
  unsigned rootClassMD = context.getMDKindID(mdName);
  llvm::MDNode *paylod = llvm::MDNode::get(context, { llvm::MDString::get(context, mdName) });
  global.setMetadata(rootClassMD, paylod);
}

void Transforms::markObjCRootClasses(llvm::Module &bitcode) {
  for (llvm::GlobalObject &global : bitcode.global_objects()) {
    if (!global.hasName()) {
      continue;
    }

    if (global.getName().startswith("OBJC_METACLASS_$")) {
      auto &metaclass = llvm::cast<llvm::GlobalVariable>(global);
      if (!metaclass.hasInitializer()) {
        markAsRoot(metaclass);
        continue;
      }
      auto *metaclassDefinition = llvm::cast<llvm::ConstantStruct>(metaclass.getInitializer());
      llvm::Constant *superclassSlot = metaclassDefinition->getAggregateElement(1);
      if (superclassSlot->isNullValue()) {
        markAsRoot(metaclass);
      }
      if (auto superclass = llvm::dyn_cast<llvm::GlobalVariable>(superclassSlot)) {
        if (superclass->hasName() && !superclass->getName().startswith("OBJC_METACLASS_$")) {
          markAsRoot(metaclass);
        }
      }
    } else if (global.getName().startswith("OBJC_CLASS_$")) {
      auto &metaclass = llvm::cast<llvm::GlobalVariable>(global);
      if (!metaclass.hasInitializer()) {
        markAsRoot(metaclass);
        continue;
      }
      auto *metaclassDefinition = llvm::cast<llvm::ConstantStruct>(metaclass.getInitializer());
      llvm::Constant *superclassSlot = metaclassDefinition->getAggregateElement(1);
      if (superclassSlot->isNullValue()) {
        markAsRoot(metaclass);
      }
    }
  }
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
    for (auto &method : traversal.objcMethods(objcClass)) {
      llvm::FunctionType *type = method.function->getFunctionType();
      assert(type->getNumParams() >= 2 &&
             "ObjC method expected to have implicit parameters (self and _cmd)");
      auto *selfType = llvm::cast<llvm::PointerType>(type->getParamType(0));
      auto *selfStruct = llvm::cast<llvm::StructType>(selfType->getPointerElementType());
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
