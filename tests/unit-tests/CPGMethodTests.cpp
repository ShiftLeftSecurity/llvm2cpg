#include "FixturePaths.h"
#include <gtest/gtest.h>
#include <llvm/IR/Instructions.h>
#include <llvm2cpg/CPG/BitcodeLoader.h>
#include <llvm2cpg/CPG/CPGMethod.h>

using namespace llvm2cpg;

/*

Given the method `test` below,current AST implementation is supposed to produce the tree that
follows.

define dso_local i32 @test(i32 %param) #0 {
entry:
  %retval = alloca i32, align 4
  %param.addr = alloca i32, align 4
  store i32 %param, i32* %param.addr, align 4
  %0 = load i32, i32* %param.addr, align 4
  %rem = srem i32 %0, 2
  %cmp = icmp eq i32 %rem, 0
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %1 = load i32, i32* %param.addr, align 4
  %mul = mul nsw i32 %1, 42
  store i32 %mul, i32* %retval, align 4
  br label %return

if.end:                                           ; preds = %entry
  store i32 0, i32* %retval, align 4
  br label %return

return:                                           ; preds = %if.end, %if.then
  %2 = load i32, i32* %retval, align 4
  ret i32 %2
}

Expected Tree:

<root>
  ret i32 %temp2
    %temp2 = load i32, i32* %retval
      %retval = alloca i32
        i32 1

  store i32 %mul, i32* %retval
    %mul = mul i32 %temp1, 42
      %temp1 = load i32, i32* %param.addr
        %param.addr = alloca i32
          i32 1
      i32 42
    %retval = alloca i32

  %cmp = icmp eq i32 %rem, 0
    %rem = srem i32 %temp0, 2
      %temp0 = load i32, i32* %param.addr
        %param.addr = alloca i32
      i32 2
    i32 0

  store i32 %0, i32* %param.addr
    i32 %0
    %param.addr = alloca i32

  store i32 0, i32* %retval
    i32 0
    %retval = alloca i32

 */

void assertNodeChildren(ASTNode *node, const std::vector<llvm::Value *> &values) {
  ASSERT_NE(node, nullptr);
  ASSERT_EQ(node->getChildren().size(), values.size());
  for (size_t i = 0; i < node->getChildren().size(); i++) {
    auto v1 = node->getChildren()[i]->getValue();
    ASSERT_NE(v1, nullptr);
    auto v2 = values[i];
    ASSERT_NE(v2, nullptr);

    ASSERT_EQ(v1, v2) << "Parent: " << valueToString(node->getValue())
                      << "\nActual: " << valueToString(v1) << "\nExpected: " << valueToString(v2);
  }
}

/// TODO: Simplify?
TEST(CPGMethod, construction) {
  llvm::LLVMContext context;
  llvm::Module module("test", context);

  auto int32Type = llvm::Type::getInt32Ty(context);
  auto int32PtrType = llvm::Type::getInt32PtrTy(context);
  auto int1Type = llvm::Type::getInt1Ty(context);
  auto voidType = llvm::Type::getVoidTy(context);

  auto functionType = llvm::FunctionType::get(int32Type, { int32Type }, false);
  auto function =
      llvm::Function::Create(functionType, llvm::Function::InternalLinkage, "test", module);

  auto entryBlock = llvm::BasicBlock::Create(context, "entry", function);
  auto ifThenBlock = llvm::BasicBlock::Create(context, "if.then", function);
  auto ifElseBlock = llvm::BasicBlock::Create(context, "if.else", function);
  auto returnBlock = llvm::BasicBlock::Create(context, "return", function);

  auto constant_0 = llvm::ConstantInt::get(int32Type, 0, true);
  auto constant_1 = llvm::ConstantInt::get(int32Type, 1, true);
  auto constant_2 = llvm::ConstantInt::get(int32Type, 2, true);
  auto constant_42 = llvm::ConstantInt::get(int32Type, 42, true);

  /// Entry Basic Block
  auto param = &*function->arg_begin();
  auto retval = new llvm::AllocaInst(int32Type, 0, "retval", entryBlock);
  auto paramAddr = new llvm::AllocaInst(int32Type, 0, "param.addr", entryBlock);
  auto storeParam = new llvm::StoreInst(param, paramAddr, entryBlock);
  auto temp0 = new llvm::LoadInst(paramAddr, "temp0", entryBlock);
  auto rem =
      llvm::BinaryOperator::Create(llvm::Instruction::SRem, temp0, constant_2, "rem", entryBlock);
  auto cmp = llvm::CmpInst::Create(llvm::Instruction::ICmp,
                                   llvm::CmpInst::Predicate::ICMP_EQ,
                                   rem,
                                   constant_0,
                                   "cmp",
                                   entryBlock);
  llvm::BranchInst::Create(ifThenBlock, ifElseBlock, cmp, entryBlock);

  /// If Then Basic Block
  auto temp1 = new llvm::LoadInst(paramAddr, "temp1", ifThenBlock);
  auto mul =
      llvm::BinaryOperator::Create(llvm::Instruction::Mul, temp1, constant_42, "mul", ifThenBlock);
  auto storeMulRetVal = new llvm::StoreInst(mul, retval, ifThenBlock);
  llvm::BranchInst::Create(returnBlock, ifThenBlock);

  /// if Else Basic Block
  auto storeConstRetVal = new llvm::StoreInst(constant_0, retval, ifElseBlock);
  llvm::BranchInst::Create(returnBlock, ifElseBlock);

  /// Return Basic Block
  auto temp2 = new llvm::LoadInst(retval, "temp2", returnBlock);
  auto ret = llvm::ReturnInst::Create(context, temp2, returnBlock);

  CPGMethod method(*function);

  std::vector<llvm::Type *> usedTypes({ int32Type, int32PtrType, int1Type, voidType });

  ASSERT_EQ(method.getTypes().size(), usedTypes.size());
  for (auto type : usedTypes) {
    ASSERT_EQ(method.getTypes().count(type), size_t(1));
  }

  auto root = method.getRoot();
  ASSERT_NE(root, nullptr);
  ASSERT_EQ(root->getValue(), nullptr);

  /// Compare Trees
  assertNodeChildren(root, { ret, storeConstRetVal, storeMulRetVal, cmp, storeParam });

  //  ret i32 %temp2
  //    %temp2 = load i32, i32* %retval
  //      %retval = alloca i32
  assertNodeChildren(method.getTree(ret), { temp2 });
  assertNodeChildren(method.getTree(temp2), { retval });
  assertNodeChildren(method.getTree(retval), { constant_1 });

  //  store i32 %mul, i32* %retval
  //    %mul = mul i32 %temp1, 42
  //      %temp1 = load i32, i32* %param.addr
  //        %param.addr = alloca i32
  //      i32 42
  //    %retval = alloca i32
  assertNodeChildren(method.getTree(storeMulRetVal), { mul, retval });
  assertNodeChildren(method.getTree(mul), { temp1, constant_42 });
  assertNodeChildren(method.getTree(temp1), { paramAddr });
  assertNodeChildren(method.getTree(paramAddr), { constant_1 });
  assertNodeChildren(method.getTree(retval), { constant_1 });

  //  %cmp = icmp eq i32 %rem, 0
  //    %rem = srem i32 %temp0, 2
  //      %temp0 = load i32, i32* %param.addr
  //        %param.addr = alloca i32
  //      i32 2
  //    i32 0
  assertNodeChildren(method.getTree(cmp), { rem, constant_0 });
  assertNodeChildren(method.getTree(rem), { temp0, constant_2 });
  assertNodeChildren(method.getTree(temp0), { paramAddr });
  assertNodeChildren(method.getTree(paramAddr), { constant_1 });

  //  store i32 %0, i32* %param.addr
  //    i32 %0
  //    %param.addr = alloca i32
  assertNodeChildren(method.getTree(storeParam), { param, paramAddr });

  //  store i32 0, i32* %retval
  //    i32 0
  //    %retval = alloca i32
  assertNodeChildren(method.getTree(storeConstRetVal), { constant_0, retval });
}

//  define dso_local i32 @hello_world() #0 {
//    ret i32 42
//  }
TEST(CPGMethod, constructionFromSimpleFunction) {
  llvm::LLVMContext context;
  llvm::Module module("test", context);

  auto int32Type = llvm::Type::getInt32Ty(context);
  auto voidType = llvm::Type::getVoidTy(context);

  auto functionType = llvm::FunctionType::get(int32Type, false);
  auto function =
      llvm::Function::Create(functionType, llvm::Function::InternalLinkage, "test", module);

  auto constant_42 = llvm::ConstantInt::get(int32Type, 42, true);
  auto block = llvm::BasicBlock::Create(context, "entry", function);
  auto ret = llvm::ReturnInst::Create(context, constant_42, block);

  CPGMethod method(*function);

  ASSERT_NE(method.getRoot(), nullptr);

  auto root = method.getRoot();
  assertNodeChildren(root, { ret });
  assertNodeChildren(method.getTree(ret), { constant_42 });

  std::vector<llvm::Type *> usedTypes({ int32Type, voidType });

  ASSERT_EQ(method.getTypes().size(), usedTypes.size());
  for (auto type : usedTypes) {
    ASSERT_EQ(method.getTypes().count(type), size_t(1));
  }
}
