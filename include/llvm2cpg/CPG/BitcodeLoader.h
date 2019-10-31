#pragma once

#include <llvm/IR/Module.h>
#include <memory>
#include <string>
#include <vector>

namespace llvm2cpg {

class CPGLogger;

class BitcodeLoader {
public:
  explicit BitcodeLoader(CPGLogger &logger);
  std::unique_ptr<llvm::Module> loadBitcode(const std::string &path, llvm::LLVMContext &context);
  std::unique_ptr<llvm::Module> loadIR(const std::string &path, llvm::LLVMContext &context);
  std::vector<std::unique_ptr<llvm::Module>> extractBitcode(const std::string &path,
                                                            llvm::LLVMContext &context);

private:
  std::unique_ptr<llvm::Module> loadBitcode(llvm::MemoryBuffer &buffer, llvm::LLVMContext &context);

  CPGLogger &logger;
};

} // namespace llvm2cpg
