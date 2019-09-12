#include "FixturePaths.h"
#include <gtest/gtest.h>
#include <llvm2cpg/CPG/BitcodeLoader.h>

using namespace llvm2cpg;

TEST(BitcodeLoader, loadBitcode_invalidPath) {
  llvm::LLVMContext context;
  BitcodeLoader loader;
  auto bitcode = loader.loadBitcode("whatever", context);
  ASSERT_EQ(bitcode.get(), nullptr);
}

TEST(BitcodeLoader, loadBitcode_validPath) {
  llvm::LLVMContext context;
  BitcodeLoader loader;
  auto bitcode = loader.loadBitcode(fixtures::hello_world_hello_world_bc_path(), context);
  bitcode->print(llvm::errs(), nullptr);

  ASSERT_NE(bitcode.get(), nullptr);
  ASSERT_STREQ(fixtures::hello_world_hello_world_bc_input_path(),
               bitcode.get()->getSourceFileName().c_str());
}