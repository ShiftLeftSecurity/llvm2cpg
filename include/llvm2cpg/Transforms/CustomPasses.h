#include <llvm/IR/PassManager.h>

namespace llvm2cpg {
namespace customPasses {
struct StripOptNonePass : public llvm::PassInfoMixin<StripOptNonePass> {
  llvm::PreservedAnalyses run(llvm::Module &M, llvm::ModuleAnalysisManager &MAM);
};

struct DemotePhiPass : public llvm::PassInfoMixin<DemotePhiPass> {
  llvm::PreservedAnalyses run(llvm::Function &function, llvm::FunctionAnalysisManager &FAM);
};

struct LoadInlinePass : public llvm::PassInfoMixin<LoadInlinePass> {
  llvm::PreservedAnalyses run(llvm::Function &function, llvm::FunctionAnalysisManager &FAM);
};

} // namespace customPasses
} // namespace llvm2cpg