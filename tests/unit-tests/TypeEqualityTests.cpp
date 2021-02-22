#include <gtest/gtest.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm2cpg/LLVMExt/TypeEquality.h>
#include <string>
#include <unordered_map>
#include <vector>

struct BasicTestInput {
  llvm::Type *type_0;
  llvm::Type *type_1;
  BasicTestInput(llvm::Type *t0, llvm::Type *t1) : type_0(t0), type_1(t1) {}

  friend std::ostream &operator<<(std::ostream &os, const BasicTestInput &parameter) {
    std::string string;
    llvm::raw_string_ostream ostream(string);
    ostream << "lhs(" << *parameter.type_0 << "), rhs(" << *parameter.type_1 << ")";
    os << ostream.str();
    return os;
  }
};

class BasicEqualityTests : public testing::TestWithParam<BasicTestInput> {};
class BasicInequalityTests : public testing::TestWithParam<BasicTestInput> {};

static llvm::LLVMContext globalContext_0;
static llvm::LLVMContext globalContext_1;

#define EQUALITY_TEST_INPUT(TypeInput)                                                             \
  BasicTestInput(llvm::Type::get##TypeInput##Ty(globalContext_0),                                  \
                 llvm::Type::get##TypeInput##Ty(globalContext_0)),                                 \
      BasicTestInput(llvm::Type::get##TypeInput##Ty(globalContext_0),                              \
                     llvm::Type::get##TypeInput##Ty(globalContext_1))

static std::vector<BasicTestInput> equalityTestInputs{
  // clang-format off
  EQUALITY_TEST_INPUT(Void),

  EQUALITY_TEST_INPUT(Label),
  EQUALITY_TEST_INPUT(Metadata),
  EQUALITY_TEST_INPUT(Token),

  EQUALITY_TEST_INPUT(Half),
  EQUALITY_TEST_INPUT(Float),
  EQUALITY_TEST_INPUT(Double),

  EQUALITY_TEST_INPUT(X86_FP80),
  EQUALITY_TEST_INPUT(X86_MMX),
  EQUALITY_TEST_INPUT(PPC_FP128),
  EQUALITY_TEST_INPUT(FP128),

  EQUALITY_TEST_INPUT(Int16),
  EQUALITY_TEST_INPUT(Int32),
  EQUALITY_TEST_INPUT(Int128),

  EQUALITY_TEST_INPUT(Int16Ptr),
  EQUALITY_TEST_INPUT(FloatPtr),
  // clang-format on

  /// Vector Type
  BasicTestInput(llvm::VectorType::get(llvm::Type::getInt16Ty(globalContext_0), 10, true),
                 llvm::VectorType::get(llvm::Type::getInt16Ty(globalContext_0), 10, true)),
  BasicTestInput(llvm::VectorType::get(llvm::Type::getInt16Ty(globalContext_0), 10, true),
                 llvm::VectorType::get(llvm::Type::getInt16Ty(globalContext_1), 10, true)),

  // Array Type
  BasicTestInput(llvm::ArrayType::get(llvm::Type::getInt16Ty(globalContext_0), 10),
                 llvm::ArrayType::get(llvm::Type::getInt16Ty(globalContext_0), 10)),
  BasicTestInput(llvm::ArrayType::get(llvm::Type::getInt16Ty(globalContext_0), 10),
                 llvm::ArrayType::get(llvm::Type::getInt16Ty(globalContext_1), 10)),

  // Function Type
  BasicTestInput(llvm::FunctionType::get(llvm::Type::getInt16Ty(globalContext_0), {}, false),
                 llvm::FunctionType::get(llvm::Type::getInt16Ty(globalContext_0), {}, false)),
  BasicTestInput(llvm::FunctionType::get(llvm::Type::getInt16Ty(globalContext_0), {}, false),
                 llvm::FunctionType::get(llvm::Type::getInt16Ty(globalContext_1), {}, false)),

  BasicTestInput(llvm::FunctionType::get(llvm::Type::getInt16Ty(globalContext_0),
                                         { llvm::Type::getInt32Ty(globalContext_0) }, false),
                 llvm::FunctionType::get(llvm::Type::getInt16Ty(globalContext_0),
                                         { llvm::Type::getInt32Ty(globalContext_0) }, false)),
  BasicTestInput(llvm::FunctionType::get(llvm::Type::getInt16Ty(globalContext_0),
                                         { llvm::Type::getInt32Ty(globalContext_0) }, false),
                 llvm::FunctionType::get(llvm::Type::getInt16Ty(globalContext_1),
                                         { llvm::Type::getInt32Ty(globalContext_1) }, false)),

  BasicTestInput(llvm::FunctionType::get(llvm::Type::getInt16Ty(globalContext_0),
                                         { llvm::Type::getInt32Ty(globalContext_0) }, true),
                 llvm::FunctionType::get(llvm::Type::getInt16Ty(globalContext_0),
                                         { llvm::Type::getInt32Ty(globalContext_0) }, true)),
  BasicTestInput(llvm::FunctionType::get(llvm::Type::getInt16Ty(globalContext_0),
                                         { llvm::Type::getInt32Ty(globalContext_0) }, true),
                 llvm::FunctionType::get(llvm::Type::getInt16Ty(globalContext_1),
                                         { llvm::Type::getInt32Ty(globalContext_1) }, true)),

  BasicTestInput(llvm::StructType::create(
                     globalContext_0, { llvm::Type::getInt32Ty(globalContext_0) }, "foo", false),
                 llvm::StructType::create(
                     globalContext_0, { llvm::Type::getInt32Ty(globalContext_0) }, "bar", false)),

  BasicTestInput(llvm::StructType::create(
                     globalContext_0, { llvm::Type::getInt32Ty(globalContext_0) }, "foo", false),
                 llvm::StructType::create(
                     globalContext_1, { llvm::Type::getInt32Ty(globalContext_1) }, "bar", false)),
};
#undef EQUALITY_TEST_INPUT

#define INEQUALITY_TEST_INPUT(TypeInput_0, TypeInput_1)                                            \
  BasicTestInput(llvm::Type::get##TypeInput_0##Ty(globalContext_0),                                \
                 llvm::Type::get##TypeInput_1##Ty(globalContext_0)),                               \
      BasicTestInput(llvm::Type::get##TypeInput_0##Ty(globalContext_0),                            \
                     llvm::Type::get##TypeInput_1##Ty(globalContext_1))

static std::vector<BasicTestInput> inequalityTestInputs{
  // clang-format off
  INEQUALITY_TEST_INPUT(Void, Label),
  INEQUALITY_TEST_INPUT(Int16, Int32),
  INEQUALITY_TEST_INPUT(X86_FP80, X86_MMX),
  INEQUALITY_TEST_INPUT(Int16Ptr, FloatPtr),
  INEQUALITY_TEST_INPUT(Half, Float),
  INEQUALITY_TEST_INPUT(Metadata, Token),
  // clang-format on

  BasicTestInput(llvm::VectorType::get(llvm::Type::getInt16Ty(globalContext_0), 9, true),
                 llvm::VectorType::get(llvm::Type::getInt16Ty(globalContext_0), 10, true)),
  BasicTestInput(llvm::VectorType::get(llvm::Type::getInt16Ty(globalContext_0), 9, true),
                 llvm::VectorType::get(llvm::Type::getInt16Ty(globalContext_1), 10, true)),

  BasicTestInput(llvm::VectorType::get(llvm::Type::getInt16Ty(globalContext_0), 10, true),
                 llvm::VectorType::get(llvm::Type::getInt16Ty(globalContext_0), 10, false)),
  BasicTestInput(llvm::VectorType::get(llvm::Type::getInt16Ty(globalContext_0), 10, true),
                 llvm::VectorType::get(llvm::Type::getInt16Ty(globalContext_1), 10, false)),

  BasicTestInput(llvm::VectorType::get(llvm::Type::getInt32Ty(globalContext_0), 10, true),
                 llvm::VectorType::get(llvm::Type::getInt16Ty(globalContext_0), 10, true)),
  BasicTestInput(llvm::VectorType::get(llvm::Type::getInt32Ty(globalContext_0), 10, true),
                 llvm::VectorType::get(llvm::Type::getInt16Ty(globalContext_1), 10, true)),

  BasicTestInput(llvm::VectorType::get(llvm::Type::getInt32PtrTy(globalContext_0), 10, true),
                 llvm::VectorType::get(llvm::Type::getInt16PtrTy(globalContext_0), 10, true)),
  BasicTestInput(llvm::VectorType::get(llvm::Type::getInt32PtrTy(globalContext_0), 10, true),
                 llvm::VectorType::get(llvm::Type::getInt16PtrTy(globalContext_1), 10, true)),

  BasicTestInput(llvm::ArrayType::get(llvm::Type::getInt16Ty(globalContext_0), 9),
                 llvm::ArrayType::get(llvm::Type::getInt16Ty(globalContext_0), 10)),
  BasicTestInput(llvm::ArrayType::get(llvm::Type::getInt16Ty(globalContext_0), 9),
                 llvm::ArrayType::get(llvm::Type::getInt16Ty(globalContext_1), 10)),

  BasicTestInput(llvm::ArrayType::get(llvm::Type::getInt32Ty(globalContext_0), 10),
                 llvm::ArrayType::get(llvm::Type::getInt16Ty(globalContext_0), 10)),
  BasicTestInput(llvm::ArrayType::get(llvm::Type::getInt32Ty(globalContext_0), 10),
                 llvm::ArrayType::get(llvm::Type::getInt16Ty(globalContext_1), 10)),

  BasicTestInput(llvm::ArrayType::get(llvm::Type::getInt32PtrTy(globalContext_0), 10),
                 llvm::ArrayType::get(llvm::Type::getInt16PtrTy(globalContext_0), 10)),
  BasicTestInput(llvm::ArrayType::get(llvm::Type::getInt32PtrTy(globalContext_0), 10),
                 llvm::ArrayType::get(llvm::Type::getInt16PtrTy(globalContext_1), 10)),

  BasicTestInput(llvm::FunctionType::get(llvm::Type::getInt16Ty(globalContext_0), {}, false),
                 llvm::FunctionType::get(llvm::Type::getInt16Ty(globalContext_0), {}, true)),
  BasicTestInput(llvm::FunctionType::get(llvm::Type::getInt16Ty(globalContext_0), {}, false),
                 llvm::FunctionType::get(llvm::Type::getInt16Ty(globalContext_1), {}, true)),

  BasicTestInput(llvm::FunctionType::get(llvm::Type::getInt16Ty(globalContext_0),
                                         { llvm::Type::getInt32Ty(globalContext_0) }, false),
                 llvm::FunctionType::get(llvm::Type::getInt16Ty(globalContext_0),
                                         { llvm::Type::getInt16Ty(globalContext_0) }, false)),

  BasicTestInput(llvm::FunctionType::get(llvm::Type::getInt16Ty(globalContext_0), {}, false),
                 llvm::FunctionType::get(llvm::Type::getInt16Ty(globalContext_0),
                                         { llvm::Type::getInt16Ty(globalContext_0) }, false)),

  BasicTestInput(llvm::FunctionType::get(llvm::Type::getInt32Ty(globalContext_0),
                                         { llvm::Type::getInt32Ty(globalContext_0) }, true),
                 llvm::FunctionType::get(llvm::Type::getInt16Ty(globalContext_0),
                                         { llvm::Type::getInt32Ty(globalContext_0) }, true)),

  BasicTestInput(llvm::StructType::create(globalContext_0,
                                          { llvm::Type::getInt32Ty(globalContext_0) }, "foo", true),
                 llvm::StructType::create(
                     globalContext_1, { llvm::Type::getInt32Ty(globalContext_1) }, "bar", false)),
  BasicTestInput(llvm::StructType::create(
                     globalContext_0, { llvm::Type::getInt16Ty(globalContext_0) }, "foo", false),
                 llvm::StructType::create(
                     globalContext_1, { llvm::Type::getInt32Ty(globalContext_1) }, "bar", false)),
};

#undef INEQUALITY_TEST_INPUT

TEST_P(BasicEqualityTests, test) {
  llvm_ext::TypesComparator comparator;
  ASSERT_TRUE(comparator.typesEqual(GetParam().type_0, GetParam().type_0));
  ASSERT_TRUE(comparator.typesEqual(GetParam().type_0, GetParam().type_1));
}

INSTANTIATE_TEST_SUITE_P(BasicTypeEqualitySuite, BasicEqualityTests,
                         testing::ValuesIn(equalityTestInputs));

TEST_P(BasicInequalityTests, test) {
  llvm_ext::TypesComparator comparator;
  std::unordered_map<std::string, size_t> opaqueStructs;
  ASSERT_FALSE(comparator.typesEqual(GetParam().type_0, GetParam().type_1));
}

INSTANTIATE_TEST_SUITE_P(BasicTypeEqualitySuite, BasicInequalityTests,
                         testing::ValuesIn(inequalityTestInputs));

TEST(ComplexTypeEquality, selfReferencingStruct) {
  llvm_ext::TypesComparator comparator;
  llvm::LLVMContext context_0;
  llvm::StructType *structType_0 = llvm::StructType::create(context_0);
  structType_0->setBody({ llvm::PointerType::get(structType_0, 0) }, false);

  llvm::LLVMContext context_1;
  llvm::StructType *structType_1 = llvm::StructType::create(context_1);
  structType_1->setBody({ llvm::PointerType::get(structType_1, 0) }, false);

  ASSERT_TRUE(comparator.typesEqual(structType_0, structType_1));
}

TEST(ComplexTypeEquality, recursiveStructReference) {
  llvm_ext::TypesComparator comparator;
  std::unordered_map<std::string, size_t> opaqueStructs;
  llvm::LLVMContext context;
  llvm::StructType *structType_0 = llvm::StructType::create(context);
  structType_0->setBody({ llvm::PointerType::get(structType_0, 0) }, false);

  llvm::StructType *structType_1 = llvm::StructType::create(context);
  structType_1->setBody({ llvm::PointerType::get(structType_1, 0) }, false);

  llvm::StructType *structType_2 = llvm::StructType::create(context);
  structType_2->setBody({ llvm::PointerType::get(structType_1, 0) }, false);

  ASSERT_FALSE(comparator.typesEqual(structType_0, structType_2));
  ASSERT_FALSE(comparator.typesEqual(structType_1, structType_2));
  ASSERT_TRUE(comparator.typesEqual(structType_0, structType_1));
}

TEST(ComplexTypeEquality, canonicalNames) {
  std::unordered_map<std::string, std::string> inputs{
    { "foo", "foo" },        { "bar", "bar" },       { "foo.0", "foo" },
    { "struct.foo", "foo" }, { "class.foo", "foo" }, { "union.foo", "foo" },
  };

  llvm::LLVMContext context;
  for (auto &pair : inputs) {
    llvm::StructType *structType = llvm::StructType::create(context, pair.first);
    ASSERT_EQ(llvm_ext::getCanonicalName(structType), pair.second);
  }
}

TEST(ComplexTypeEquality, opaqueStructs) {
  llvm_ext::TypesComparator comparator;
  llvm::LLVMContext context_0;
  llvm::StructType *structType_0_0 = llvm::StructType::create(context_0, "foo");
  llvm::StructType *structType_0_1 = llvm::StructType::create(context_0, "bar");
  llvm::StructType *structType_0_2 = llvm::StructType::create(context_0, "foo");

  llvm::LLVMContext context_1;
  llvm::StructType *structType_1_0 = llvm::StructType::create(context_1, "foo");
  llvm::StructType *structType_1_1 = llvm::StructType::create(context_1, "bar");
  llvm::StructType *structType_1_2 = llvm::StructType::create(context_1, "foo");

  ASSERT_TRUE(structType_0_0->isOpaque());
  ASSERT_TRUE(structType_0_1->isOpaque());
  ASSERT_TRUE(structType_0_2->isOpaque());

  ASSERT_TRUE(structType_1_0->isOpaque());
  ASSERT_TRUE(structType_1_1->isOpaque());
  ASSERT_TRUE(structType_1_2->isOpaque());

  ASSERT_TRUE(comparator.typesEqual(structType_0_0, structType_0_0));
  ASSERT_TRUE(comparator.typesEqual(structType_0_0, structType_0_2));
  ASSERT_FALSE(comparator.typesEqual(structType_0_0, structType_0_1));

  ASSERT_TRUE(comparator.typesEqual(structType_0_0, structType_1_0));
  ASSERT_TRUE(comparator.typesEqual(structType_0_1, structType_1_1));
  ASSERT_TRUE(comparator.typesEqual(structType_0_2, structType_1_2));
  ASSERT_FALSE(comparator.typesEqual(structType_0_0, structType_1_1));
}

TEST(ComplexTypeEquality, nestedOpaqueStructs) {
  llvm_ext::TypesComparator comparator;
  llvm::LLVMContext context_0;
  llvm::StructType *opaque_0 = llvm::StructType::create(context_0, "foo");
  llvm::StructType *opaque_1 = llvm::StructType::create(context_0, "bar");

  llvm::LLVMContext context_1;
  llvm::StructType *opaque_2 = llvm::StructType::create(context_1, "foo");
  llvm::StructType *opaque_3 = llvm::StructType::create(context_1, "foo");

  ASSERT_TRUE(opaque_0->isOpaque());
  ASSERT_TRUE(opaque_1->isOpaque());
  ASSERT_TRUE(opaque_2->isOpaque());
  ASSERT_TRUE(opaque_3->isOpaque());

  llvm::StructType *wrapper_0 =
      llvm::StructType::create(context_0, { llvm::PointerType::get(opaque_0, 0) });
  llvm::StructType *wrapper_1 =
      llvm::StructType::create(context_0, { llvm::PointerType::get(opaque_1, 0) });

  llvm::StructType *wrapper_2 =
      llvm::StructType::create(context_1, { llvm::PointerType::get(opaque_2, 0) });
  llvm::StructType *wrapper_3 =
      llvm::StructType::create(context_1, { llvm::PointerType::get(opaque_3, 0) });

  ASSERT_TRUE(comparator.typesEqual(wrapper_0, wrapper_2));
  ASSERT_TRUE(comparator.typesEqual(wrapper_0, wrapper_3));
  ASSERT_TRUE(comparator.typesEqual(wrapper_2, wrapper_3));
  ASSERT_TRUE(comparator.typesEqual(wrapper_1, wrapper_1));

  ASSERT_FALSE(comparator.typesEqual(wrapper_1, wrapper_0));
  ASSERT_FALSE(comparator.typesEqual(wrapper_1, wrapper_2));
  ASSERT_FALSE(comparator.typesEqual(wrapper_1, wrapper_3));
}

TEST(ComplexTypeEquality, nestedUnnamedOpaqueStructs) {
  llvm_ext::TypesComparator comparator;
  llvm::LLVMContext context;
  llvm::StructType *opaque_0 = llvm::StructType::create(context);
  llvm::StructType *opaque_1 = llvm::StructType::create(context);
  llvm::StructType *opaque_2 = llvm::StructType::create(context, "named");

  ASSERT_TRUE(opaque_0->isOpaque());
  ASSERT_TRUE(opaque_1->isOpaque());
  ASSERT_TRUE(opaque_2->isOpaque());

  llvm::StructType *wrapper_0 =
      llvm::StructType::create(context, { llvm::PointerType::get(opaque_0, 0) });
  llvm::StructType *wrapper_1 =
      llvm::StructType::create(context, { llvm::PointerType::get(opaque_1, 0) });
  llvm::StructType *wrapper_2 =
      llvm::StructType::create(context, { llvm::PointerType::get(opaque_0, 0) });
  llvm::StructType *wrapper_3 =
      llvm::StructType::create(context, { llvm::PointerType::get(opaque_2, 0) });

  ASSERT_FALSE(comparator.typesEqual(wrapper_0, wrapper_1));
  ASSERT_FALSE(comparator.typesEqual(wrapper_0, wrapper_3));
  ASSERT_FALSE(comparator.typesEqual(wrapper_1, wrapper_2));
  ASSERT_FALSE(comparator.typesEqual(wrapper_1, wrapper_3));
  ASSERT_FALSE(comparator.typesEqual(wrapper_2, wrapper_3));

  ASSERT_TRUE(comparator.typesEqual(wrapper_0, wrapper_2));
}
