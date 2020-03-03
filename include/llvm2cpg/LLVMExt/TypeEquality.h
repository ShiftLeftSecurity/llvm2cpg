#pragma once

#include <string>
#include <unordered_map>

namespace llvm {
class Type;
class StructType;
} // namespace llvm

namespace llvm_ext {

std::string getCanonicalName(const llvm::StructType *type);

class TypesComparator {
public:
  bool typesEqual(const llvm::Type *lhs, const llvm::Type *rhs);

private:
  size_t computeHash(const llvm::Type *type);

  std::unordered_map<std::string, size_t> opaqueStructs;
  std::unordered_map<const llvm::StructType *, size_t> unnamedStructs;
  std::unordered_map<const llvm::Type *, size_t> hashCache;
};

} // namespace llvm_ext
