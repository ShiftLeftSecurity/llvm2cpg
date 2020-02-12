#include "llvm2cpg/LLVMExt/TypeEquality.h"
#include <functional>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Type.h>
#include <stack>
#include <unordered_map>
#include <vector>

static size_t getTypeArity(const llvm::Type *type) {
  switch (type->getTypeID()) {
  case llvm::Type::VoidTyID:
  case llvm::Type::HalfTyID:
  case llvm::Type::FloatTyID:
  case llvm::Type::DoubleTyID:
  case llvm::Type::X86_FP80TyID:
  case llvm::Type::FP128TyID:
  case llvm::Type::PPC_FP128TyID:
  case llvm::Type::LabelTyID:
  case llvm::Type::MetadataTyID:
  case llvm::Type::X86_MMXTyID:
  case llvm::Type::TokenTyID:
    return 0;

  case llvm::Type::IntegerTyID:
    return 1;
  case llvm::Type::ArrayTyID:
    return 2;
  case llvm::Type::PointerTyID:
    return 1;
  case llvm::Type::VectorTyID:
    return 3;

  case llvm::Type::FunctionTyID:
    return type->getFunctionNumParams() + 3;
  case llvm::Type::StructTyID: {
    if (llvm::cast<llvm::StructType>(type)->isOpaque()) {
      return 1;
    }
    return type->getStructNumElements() + 3;
  }
  }
}

static size_t getTypeId(const llvm::Type *type) {
  if (type->isStructTy() && llvm::cast<llvm::StructType>(type)->isOpaque()) {
    /// We treat opaque structs a separate type and therefore assigning an arbitrary ID to it
    return 42;
  }
  return type->getTypeID();
}

static std::vector<size_t>
constructTypeVector(const llvm::Type *topType,
                    std::unordered_map<std::string, size_t> &opaqueStructs) {
  std::vector<size_t> result;

  // Using stack for pre-order traversal
  std::stack<const llvm::Type *> state({ topType });

  std::unordered_map<const llvm::Type *, size_t> visitedStructs;
  size_t totalArity = 1;

  while (!state.empty()) {
    const llvm::Type *type = state.top();
    state.pop();
    size_t arity = getTypeArity(type);
    result.push_back(getTypeId(type));
    result.push_back(arity);
    totalArity += arity + 1;

    switch (type->getTypeID()) {
    case llvm::Type::VoidTyID:
    case llvm::Type::HalfTyID:
    case llvm::Type::FloatTyID:
    case llvm::Type::DoubleTyID:
    case llvm::Type::X86_FP80TyID:
    case llvm::Type::FP128TyID:
    case llvm::Type::PPC_FP128TyID:
    case llvm::Type::LabelTyID:
    case llvm::Type::MetadataTyID:
    case llvm::Type::X86_MMXTyID:
    case llvm::Type::TokenTyID:
      continue;
    case llvm::Type::IntegerTyID: {
      result.push_back(type->getIntegerBitWidth());
    } break;
    case llvm::Type::FunctionTyID: {
      result.push_back(type->getFunctionNumParams());
      result.push_back(type->isFunctionVarArg());
      unsigned paramSize = type->getFunctionNumParams();
      for (unsigned i = 0; i < paramSize; i++) {
        state.push(type->getFunctionParamType(paramSize - i - 1));
      }
      state.push(llvm::cast<llvm::FunctionType>(type)->getReturnType());
    } break;
    case llvm::Type::StructTyID: {
      const auto *structType = llvm::cast<llvm::StructType>(type);
      if (structType->isOpaque()) {
        std::string canonicalName = llvm_ext::getCanonicalName(structType);
        if (!opaqueStructs.count(canonicalName)) {
          opaqueStructs.insert(std::make_pair(canonicalName, opaqueStructs.size()));
        }
        result.push_back(opaqueStructs[canonicalName]);
      } else {
        unsigned elementsSize = type->getStructNumElements();
        result.push_back(elementsSize);
        result.push_back(llvm::cast<llvm::StructType>(type)->isPacked());

        bool structVisited = visitedStructs.count(type) != 0;
        if (!structVisited) {
          visitedStructs.insert(std::make_pair(type, visitedStructs.size()));
        }
        result.push_back(visitedStructs[type]);

        if (structVisited) {
          for (unsigned i = 0; i < elementsSize; i++) {
            result.push_back(0);
          }
        } else {
          for (unsigned i = 0; i < elementsSize; i++) {
            state.push(type->getStructElementType(elementsSize - i - 1));
          }
        }
      }
    } break;
    case llvm::Type::ArrayTyID: {
      result.push_back(type->getArrayNumElements());
      state.push(type->getArrayElementType());
    } break;
    case llvm::Type::PointerTyID: {
      state.push(type->getPointerElementType());
    } break;
    case llvm::Type::VectorTyID: {
      result.push_back(type->getVectorNumElements());
      result.push_back(type->getVectorIsScalable());
      state.push(type->getVectorElementType());
    } break;
    }
  }

  assert(result.size() == totalArity);

  return result;
}

std::string llvm_ext::getCanonicalName(const llvm::StructType *type) {
  assert(type);
  assert(type->hasName() && "Not supposed to extract canonical name from an anonymous struct");
  llvm::StringRef name = type->getStructName();
  name.consume_front("class.");
  name.consume_front("struct.");
  name.consume_front("union.");
  return name.substr(0, name.find_first_of('.', 0));
}

bool llvm_ext::typesEqual(const llvm::Type *lhs, const llvm::Type *rhs,
                          std::unordered_map<std::string, size_t> &opaqueStructs) {
  if (rhs == lhs) {
    return true;
  }

  std::vector<size_t> lhsVector = constructTypeVector(lhs, opaqueStructs);
  std::vector<size_t> rhsVector = constructTypeVector(rhs, opaqueStructs);
  return lhsVector == rhsVector;
}

static size_t computeTypeHash(const llvm::Type *type,
                              std::unordered_map<std::string, size_t> &opaqueStructs) {
  std::vector<size_t> typeVector = constructTypeVector(type, opaqueStructs);
  size_t seed = 0;
  for (auto &value : typeVector) {
    /// Inspired by the boost::hash_combine
    std::hash<size_t> hasher;
    seed ^= hasher(value) + 0x9e3779b9 + (seed << size_t(6)) + (seed >> size_t(2));
  }
  return seed;
}

bool llvm_ext::TypesComparator::typesEqual(const llvm::Type *lhs, const llvm::Type *rhs) {
  if (hashCache.count(lhs) == 0) {
    hashCache.insert(std::make_pair(lhs, computeTypeHash(lhs, opaqueStructs)));
  }
  if (hashCache.count(rhs) == 0) {
    hashCache.insert(std::make_pair(rhs, computeTypeHash(rhs, opaqueStructs)));
  }
  return hashCache[lhs] == hashCache[rhs];
}
