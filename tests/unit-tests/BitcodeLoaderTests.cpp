#include "FixturePaths.h"
#include <gtest/gtest.h>
#include <llvm2cpg/CPG/BitcodeLoader.h>
#include <llvm2cpg/Logger/CPGLogger.h>

using namespace llvm2cpg;

TEST(BitcodeLoader, loadBitcode_invalidPath) {
  llvm::LLVMContext context;
  CPGLogger logger;
  BitcodeLoader loader(logger);
  auto bitcode = loader.loadBitcode("whatever", context);
  ASSERT_EQ(bitcode.get(), nullptr);
}

TEST(BitcodeLoader, loadBitcode_validPath) {
  llvm::LLVMContext context;
  CPGLogger logger;
  BitcodeLoader loader(logger);
  auto bitcode = loader.loadBitcode(fixtures::return_constant_c_bc_output_path(), context);

  ASSERT_NE(bitcode.get(), nullptr);
  ASSERT_STREQ(fixtures::return_constant_c_bc_input_path(),
               bitcode.get()->getSourceFileName().c_str());
}
