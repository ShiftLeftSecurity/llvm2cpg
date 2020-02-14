#include "FixturePaths.h"
#include <gtest/gtest.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm2cpg/CPG/BitcodeLoader.h>
#include <llvm2cpg/CPG/CPGMethod.h>
#include <llvm2cpg/Logger/CPGLogger.h>

using namespace llvm2cpg;

TEST(CPGMethod, basicProperties) {
  llvm::LLVMContext context;
  CPGLogger logger;
  BitcodeLoader loader(logger);
  auto bitcode = loader.loadBitcode(fixtures::return_constant_c_bc_output_path());
  ASSERT_NE(bitcode.get(), nullptr);

  auto &function = *bitcode->functions().begin();
  CPGMethod method(function);
  ASSERT_FALSE(method.isExternal());
}

static llvm::Function *createTestFunction(llvm::Module &module, llvm::FunctionType *functionType) {
  llvm::Function::LinkageTypes linkage = llvm::Function::InternalLinkage;
  return llvm::Function::Create(functionType, linkage, "test", module);
}

TEST(CPGMethod, arguments) {
  llvm::LLVMContext context;
  llvm::Module module("test", context);

  auto int32Type = llvm::Type::getInt32Ty(context);
  auto functionType = llvm::FunctionType::get(int32Type, { int32Type, int32Type }, false);
  auto function = createTestFunction(module, functionType);

  CPGMethod method(*function);
  ASSERT_EQ(method.getArguments().size(), size_t(2));
  auto arg0 = method.getArguments().front();
  auto arg1 = method.getArguments().back();
  ASSERT_STREQ(arg0->getName().str().c_str(), "arg");
  ASSERT_STREQ(arg1->getName().str().c_str(), "arg1");
}

TEST(CPGMethod, localVariables) {
  llvm::LLVMContext context;
  llvm::Module module("test", context);

  auto int32Type = llvm::Type::getInt32Ty(context);
  auto functionType = llvm::FunctionType::get(int32Type, { int32Type }, false);
  auto function = createTestFunction(module, functionType);
  auto entryBlock = llvm::BasicBlock::Create(context, "entry", function);

  llvm::IRBuilder<> builder(entryBlock);

  auto alloca = builder.CreateAlloca(int32Type);
  builder.CreateStore(function->arg_begin(), alloca);
  auto load = builder.CreateLoad(alloca);
  builder.CreateRet(load);

  CPGMethod method(*function);
  ASSERT_EQ(method.getLocalVariables().size(), size_t(2));

  auto localVar = method.getLocalVariables().front();
  ASSERT_STREQ(localVar->getName().str().c_str(), "local");
  auto tempVar = method.getLocalVariables().back();
  ASSERT_STREQ(tempVar->getName().str().c_str(), "tmp");
}