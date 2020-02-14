#pragma once

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <memory>
#include <string>
#include <vector>

namespace llvm2cpg {

class CPGLogger;

class BitcodeLoader {
public:
  explicit BitcodeLoader(CPGLogger &logger);
  std::unique_ptr<llvm::Module> loadBitcode(const std::string &path);
  std::unique_ptr<llvm::Module> loadIR(const std::string &path);
  std::vector<std::unique_ptr<llvm::Module>> extractBitcode(const std::string &path);

private:
  std::unique_ptr<llvm::Module> loadBitcode(llvm::MemoryBuffer &buffer);
  std::vector<std::unique_ptr<llvm::LLVMContext>> contexts;

  CPGLogger &logger;
};

} // namespace llvm2cpg
