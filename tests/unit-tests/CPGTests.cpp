#include <gtest/gtest.h>
#include <llvm2cpg/CPG/BitcodeLoader.h>
#include <llvm2cpg/CPG/CPG.h>

#include "FixturePaths.h"

using namespace llvm2cpg;

TEST(CPG, empty) {
  CPG cpg;
  ASSERT_TRUE(cpg.getFiles().empty());
}

TEST(CPG, addBitcode) {
  llvm::LLVMContext context;
  BitcodeLoader loader;
  auto bitcode = loader.loadBitcode(fixtures::hello_world_c_bc_output_path(), context);
  ASSERT_NE(bitcode.get(), nullptr);

  CPG cpg;
  cpg.addBitcode(bitcode.get());
  ASSERT_EQ(cpg.getFiles().size(), size_t(1));
  auto &file = cpg.getFiles().front();
  ASSERT_EQ(file.getName(), fixtures::hello_world_c_bc_input_path());
  ASSERT_EQ(file.getMethods().size(), size_t(1));
  ASSERT_EQ(file.getTypes().size(), size_t(2));
  auto &method = file.getMethods().front();
  ASSERT_STREQ(method.getName().c_str(), "hello_world");
  ASSERT_FALSE(method.isExternal());
}