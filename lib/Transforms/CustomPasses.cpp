#include "llvm2cpg/Transforms/CustomPasses.h"
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/Transforms/Utils/Local.h>

llvm::PreservedAnalyses
llvm2cpg::customPasses::StripOptNonePass::run(llvm::Module &M, llvm::ModuleAnalysisManager &MAM) {
  for (llvm::Function &F : M) {
    F.removeFnAttr(llvm::Attribute::AttrKind::OptimizeNone);
  }
  return llvm::PreservedAnalyses::none();
}

llvm::PreservedAnalyses
llvm2cpg::customPasses::DemotePhiPass::run(llvm::Function &function,
                                           llvm::FunctionAnalysisManager &FAM) {
  std::vector<llvm::PHINode *> worklist;
  for (llvm::Instruction &instruction : llvm::instructions(function)) {
    if (auto phi = llvm::dyn_cast<llvm::PHINode>(&instruction)) {
      worklist.push_back(phi);
    }
  }

  for (llvm::PHINode *phi : worklist) {
    llvm::DemotePHIToStack(phi);
  }
  if (worklist.size() > 0) {
    return llvm::PreservedAnalyses::none();
  }
  return llvm::PreservedAnalyses::all();
}

std::pair<int, llvm::Instruction *>
validUntil(llvm::Instruction *inst, unsigned inlineMD,
           llvm::DenseMap<llvm::Instruction *, int> *firstSpoiler) {
  std::pair<int, llvm::Instruction *> res = std::make_pair(1 << 30, nullptr);
  auto baseval = firstSpoiler->find(inst);
  if (baseval != firstSpoiler->end() && baseval->second < res.first) {
    res = std::make_pair(baseval->second, inst);
  }
  for (llvm::Use &use : inst->operands()) {
    if (auto useInst = llvm::dyn_cast<llvm::Instruction>(use.get())) {
      if (useInst->getMetadata(inlineMD)) {
        auto recurse = validUntil(useInst, inlineMD, firstSpoiler);
        if (recurse.first < res.first) {
          res = recurse;
        }
      }
    }
  }
  return res;
}

int usedUntil(llvm::Instruction *inst, unsigned inlineMD,
              llvm::DenseMap<llvm::Instruction *, int> *instNumber) {
  int res = -1;
  for (llvm::User *user : inst->users()) {
    if (auto userInst = llvm::dyn_cast<llvm::Instruction>(user)) {
      if (llvm::isa<llvm::DbgInfoIntrinsic>(userInst)) {
        continue;
      }
      auto baseval = instNumber->find(userInst);
      if (baseval != instNumber->end()) {
        res = std::max(res, baseval->second);
      }
      if (userInst->getMetadata(inlineMD)) {
        res = std::max(res, usedUntil(userInst, inlineMD, instNumber));
      }
    }
  }
  return res;
}

bool isUsedoutsideBB(llvm::Instruction *inst, unsigned inlineMD) {
  llvm::BasicBlock *parentBB = inst->getParent();
  for (llvm::User *user : inst->users()) {
    if (auto userInst = llvm::dyn_cast<llvm::Instruction>(user)) {
      if (userInst->getParent() != parentBB) {
        if (llvm::isa<llvm::DbgInfoIntrinsic>(userInst))
          continue;
        return true;
      }
      if (userInst->getMetadata(inlineMD)) {
        if (isUsedoutsideBB(userInst, inlineMD))
          return true;
      }
    }
  }
  return false;
}

llvm::PreservedAnalyses
llvm2cpg::customPasses::LoadInlinePass::run(llvm::Function &function,
                                            llvm::FunctionAnalysisManager &FAM) {
  llvm::LLVMContext &ctx = function.getContext();
  llvm::AAResults &AA = FAM.getResult<llvm::AAManager>(function);
  llvm::MDNode *emptyTuple = llvm::MDNode::get(ctx, {});

  unsigned inlineMD = ctx.getMDKindID("shiftleft.inline");

  for (llvm::BasicBlock &block : function) {
    for (llvm::Instruction &inst : block) {
      // we force-inline GEP and pointer-to-pointer casts
      switch (inst.getOpcode()) {
      case llvm::Instruction::BitCast: {
        auto cast = llvm::dyn_cast<llvm::BitCastInst>(&inst);
        if (!cast->getDestTy()->isPointerTy() || !cast->getSrcTy()->isPointerTy())
          break;
      }
      case llvm::Instruction::AddrSpaceCast:
      case llvm::Instruction::GetElementPtr:
        inst.setMetadata(inlineMD, emptyTuple);
        break;
      case llvm::Instruction::PtrToInt:
      case llvm::Instruction::IntToPtr:
      // if we want to do ptrtoint/intttoptr in the future, then we'll also need to handle integer
      // arithmetic
      default:
        break;
      }
    }
  }
  // cf Sink.cpp for heuristics regarding sizes.
  llvm::DenseMap<llvm::Instruction *, int> firstSpoiler;
  llvm::DenseMap<llvm::Instruction *, int> instNumber;
  llvm::SmallPtrSet<llvm::LoadInst *, 16> unspoilt;
  llvm::SmallVector<llvm::LoadInst *, 16> despoil_cache;
  std::vector<std::pair<int, llvm::Instruction *>> toInline;
  // for better debug, let's also collect a map inst-num -> inst.
  std::vector<llvm::Instruction *> numToInst;

  for (llvm::BasicBlock &block : function) {
    firstSpoiler.clear();
    instNumber.clear();
    unspoilt.clear();
    despoil_cache.clear();
    toInline.clear();

    int idx = 0;
    for (llvm::Instruction &inst : block) {
      instNumber.insert(std::make_pair(&inst, idx));
      numToInst.push_back(&inst);
      if (auto load = llvm::dyn_cast<llvm::LoadInst>(&inst)) {
        if (!isUsedoutsideBB(load, inlineMD) &&
            !load->mayWriteToMemory()) { // don't inline volatile loads
          unspoilt.insert(load);
        }
      }
      if (inst.mayWriteToMemory()) {
        // Per devdocs, the unspoilt.erase shouldn't invalidate the iterator.
        // I don't trust it.
        for (llvm::LoadInst *load : unspoilt) {
          llvm::MemoryLocation Loc = llvm::MemoryLocation::get(load);
          if (llvm::isModSet(AA.getModRefInfo(&inst, Loc))) {
            firstSpoiler.insert(std::make_pair(load, idx));
            despoil_cache.push_back(load);
          }
        }
        if (despoil_cache.size() > 0) {
          for (auto load : despoil_cache) {
            unspoilt.erase(load);
          }
          despoil_cache.clear();
        }
      }
      idx++;
    }
    for (llvm::LoadInst *load : unspoilt) {
      firstSpoiler.insert(std::make_pair(load, idx));
    }
    /*
    Consider the following:
    int foo(int **x, int ** y){
      int res = x[2][3];
      y[17] = x[1];
      return res;
    }
    The first line is a two-deep load chain that is potentially aliased by the assignment (no
    restrict keyword). Hence, we can inline one of the two loads, but not both.

    Our choice. Here we just use canonical ordering: sort by instnum, then memory address of
    instruction. Since no two instructions can end up with the same number, this should be
    reproducible.
    */
    for (std::pair<llvm::Instruction *, int> pp : firstSpoiler) {
      int instnum = instNumber.find(pp.first)->second;
      toInline.push_back(std::make_pair(instnum, pp.first));
    }
    std::sort(toInline.begin(), toInline.end());

    for (std::pair<int, llvm::Instruction *> pp : toInline) {
      llvm::Instruction *inst = pp.second;
      auto vUPair = validUntil(inst, inlineMD, &firstSpoiler);
      int vU = vUPair.first;
      int uU = usedUntil(inst, inlineMD, &instNumber);
      if (uU <= vU) {
        inst->setMetadata(inlineMD, emptyTuple);
      } else {
        // explicitly mark as noinline for debug purposes?
        // problem: ValueAsMetadata produces badref for stores.
        // skip for now.
        /*
        auto aliasingStore = numToInst[vU];
        auto aliasingSrc = vUPair.second;

        auto mdX =  llvm::MDNode::get(ctx, {llvm::ValueAsMetadata::get(aliasingStore),
        llvm::ValueAsMetadata::get(aliasingSrc)}); inst->setMetadata(noInlineMD, mdX);
        llvm::errs() << (llvm::Instruction&)*inst << "\n";
        llvm::errs() << (llvm::MDTuple&)*mdX << " vu " << vUPair.first << " uu " << uU << "\n";
        llvm::errs() << (llvm::Instruction&)*aliasingStore<< "\n";
        llvm::errs() << (llvm::Instruction&)*aliasingSrc << "\n";
        */
      }
    }
  }

  return llvm::PreservedAnalyses::all();
}
