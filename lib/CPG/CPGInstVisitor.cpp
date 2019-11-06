#include "CPGInstVisitor.h"
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Metadata.h>

using namespace llvm2cpg;

static void setNameIfEmpty(llvm::Value *value, const std::string &name) {
  if (!value->hasName()) {
    value->setName(name);
  }
}

CPGInstVisitor::CPGInstVisitor(std::vector<llvm::Value *> &arguments,
                               std::vector<llvm::Value *> &variables)
    : arguments(arguments), variables(variables), formalArguments() {}

/*
Sets the names of locals / temps and function arguments, and collects arguments.
The general idea is:
1. auto-generate names where llvm has none.
2. When finding debug-declare / debug-value / etc annotations, overwrite the name with the debug
name. Hence, latest name wins.
3. If present, use debug info to figure out parameter names, possibly overwriting the llvm or debug
names
*/
void CPGInstVisitor::run(llvm::Function &function) {

  for (auto &arg : function.args()) {
    setNameIfEmpty(&arg, "arg");
    arguments.push_back(&arg);
  }

  for (llvm::BasicBlock &bb : function) {
    for (llvm::Instruction &inst : bb) {
      llvm::InstVisitor<CPGInstVisitor, void>::visit(inst);
    }
  }

  if (llvm::DISubprogram *md = function.getSubprogram()) {
    auto nodes = md->getRetainedNodes();
    for (llvm::DINode *node : nodes) {
      if (llvm::DILocalVariable *local = llvm::dyn_cast<llvm::DILocalVariable>(node)) {
        if (local->isParameter()) {
          formalArguments.push_back(local);
        }
      }
    }
    for (auto formalArg : formalArguments) {
      if (!formalArg->getArg() || formalArg->getArg() > arguments.size()) {
        /* LLVM and debug info disagree about the number of arguments to the function.
          This means that we cannot reasonably attach names to the llvm-argument.

          This can happen in cases where arguments have non-trivial type: Then, clang tends to
          rewrite into a normalized ABI-dependent form.

          Most important example is singleton arguments (zero-size struct).

          TODO: log this fact?
        */
        return;
      }
    }
    for (auto formalArg : formalArguments) {
      arguments[formalArg->getArg() - 1]->setName(formalArg->getName() + ".arg");
    }
  }
}

void CPGInstVisitor::visitDbgVariableIntrinsic(llvm::DbgVariableIntrinsic &instruction) {
  auto local = instruction.getVariable();
  auto src_local_name = local->getName();
  bool isComplex = instruction.getExpression()->isComplex();
  bool isptr = instruction.isAddressOfVariable();
  llvm::Value *ref = instruction.getVariableLocation();
  if (llvm::isa<llvm::Function>(ref)) {
    return;
  }
  if (local->isParameter()) {
    formalArguments.push_back(local);
    if (llvm::isa<llvm::Argument>(ref)) {
      return;
    }
  }
  ref->setName(src_local_name + (isComplex ? ".derived" : "") + (isptr ? ".addr" : ""));
}

void CPGInstVisitor::visitInstruction(llvm::Instruction &instruction) {
  if (!instruction.getType()->isVoidTy()) {
    addTempVariable(&instruction);
  }
}

void CPGInstVisitor::visitAllocaInst(llvm::AllocaInst &value) {
  addLocalVariable(&value);
}

void CPGInstVisitor::visitPHINode(llvm::PHINode &instruction) {
  llvm::report_fatal_error("PHI nodes should be destructed before CPG emission");
}

void CPGInstVisitor::addLocalVariable(llvm::Value *value) {
  setNameIfEmpty(value, "local");
  variables.push_back(value);
}

void CPGInstVisitor::addTempVariable(llvm::Value *value) {
  setNameIfEmpty(value, "tmp");
  variables.push_back(value);
}
