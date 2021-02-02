#pragma once

#include <string>

namespace llvm {
class AtomicRMWInst;
class BinaryOperator;
class CmpInst;
class CastInst;
class UnaryOperator;
} // namespace llvm

namespace llvm2cpg {

std::string binaryOperatorName(const llvm::BinaryOperator *instruction);
std::string binaryOperatorNameShort(const llvm::BinaryOperator *instruction);
std::string comparisonOperatorName(const llvm::CmpInst *instruction);
std::string comparisonOperatorNameShort(const llvm::CmpInst *instruction);
std::string castOperatorName(const llvm::CastInst *instruction);
std::string unaryOperatorName(const llvm::UnaryOperator *instruction);
std::string atomicOperatorName(const llvm::AtomicRMWInst *instruction);

} // namespace llvm2cpg
