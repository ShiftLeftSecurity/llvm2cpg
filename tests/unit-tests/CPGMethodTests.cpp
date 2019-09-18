#include "FixturePaths.h"
#include <gtest/gtest.h>
#include <llvm/IR/Instructions.h>
#include <llvm2cpg/CPG/BitcodeLoader.h>
#include <llvm2cpg/CPG/CPGMethod.h>

using namespace llvm2cpg;

TEST(CPGMethod, basicProperties) {
  llvm::LLVMContext context;
  BitcodeLoader loader;
  auto bitcode = loader.loadBitcode(fixtures::return_constant_c_bc_output_path(), context);
  ASSERT_NE(bitcode.get(), nullptr);

  auto &function = *bitcode->functions().begin();
  CPGMethod method(function);
  ASSERT_STREQ(method.getName().c_str(), "basic_c_support");
  ASSERT_FALSE(method.isExternal());
}
