#include "llvm2cpg/CPG/CPGOperatorNames.h"
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>

std::string llvm2cpg::binaryOperatorName(const llvm::BinaryOperator *instruction) {
  switch (instruction->getOpcode()) {
  case llvm::Instruction::Add:
  case llvm::Instruction::FAdd:
    return "<operator>.addition";

  case llvm::Instruction::Sub:
  case llvm::Instruction::FSub:
    return "<operator>.subtraction";

  case llvm::Instruction::Mul:
  case llvm::Instruction::FMul:
    return "<operator>.multiplication";

  case llvm::Instruction::UDiv:
  case llvm::Instruction::SDiv:
  case llvm::Instruction::FDiv:
    return "<operator>.division";

  case llvm::Instruction::URem:
  case llvm::Instruction::SRem:
  case llvm::Instruction::FRem:
    return "<operator>.modulo";

  case llvm::Instruction::Shl:
    return "<operator>.shiftLeft";
  case llvm::Instruction::LShr:
    return "<operator>.logicalShiftRight";
  case llvm::Instruction::AShr:
    return "<operator>.arithmeticShiftRight";

  case llvm::Instruction::And:
    return "<operator>.and";
  case llvm::Instruction::Or:
    return "<operator>.or";
  case llvm::Instruction::Xor:
    return "<operator>.xor";

  case llvm::Instruction::BinaryOpsEnd:
    return instruction->getOpcodeName();
  }
}

std::string llvm2cpg::binaryOperatorNameShort(const llvm::BinaryOperator *instruction) {
  switch (instruction->getOpcode()) {
  case llvm::Instruction::Add:
  case llvm::Instruction::FAdd:
    return "+";

  case llvm::Instruction::Sub:
  case llvm::Instruction::FSub:
    return "-";

  case llvm::Instruction::Mul:
  case llvm::Instruction::FMul:
    return "*";

  case llvm::Instruction::UDiv:
  case llvm::Instruction::SDiv:
  case llvm::Instruction::FDiv:
    return "/";

  case llvm::Instruction::URem:
  case llvm::Instruction::SRem:
  case llvm::Instruction::FRem:
    return "%";

  case llvm::Instruction::Shl:
    return "<<";
  case llvm::Instruction::LShr:
  case llvm::Instruction::AShr:
    return ">>";

  case llvm::Instruction::And:
    return "&&";
  case llvm::Instruction::Or:
    return "||";
  case llvm::Instruction::Xor:
    return "^";

  case llvm::Instruction::BinaryOpsEnd:
    return instruction->getOpcodeName();
  }
}

std::string llvm2cpg::comparisonOperatorName(const llvm::CmpInst *instruction) {
  switch (instruction->getPredicate()) {
  case llvm::CmpInst::FCMP_OEQ:
  case llvm::CmpInst::FCMP_UEQ:
  case llvm::CmpInst::ICMP_EQ:
    return "<operator>.equals";

  case llvm::CmpInst::FCMP_ONE:
  case llvm::CmpInst::FCMP_UNE:
  case llvm::CmpInst::ICMP_NE:
    return "<operator>.notEquals";

  case llvm::CmpInst::FCMP_OGT:
  case llvm::CmpInst::FCMP_UGT:
  case llvm::CmpInst::ICMP_UGT:
  case llvm::CmpInst::ICMP_SGT:
    return "<operator>.greaterThan";

  case llvm::CmpInst::FCMP_OLT:
  case llvm::CmpInst::FCMP_ULT:
  case llvm::CmpInst::ICMP_ULT:
  case llvm::CmpInst::ICMP_SLT:
    return "<operator>.lessThan";

  case llvm::CmpInst::ICMP_UGE:
  case llvm::CmpInst::FCMP_OGE:
  case llvm::CmpInst::FCMP_UGE:
  case llvm::CmpInst::ICMP_SGE:
    return "<operator>.greaterEqualsThan";

  case llvm::CmpInst::FCMP_OLE:
  case llvm::CmpInst::FCMP_ULE:
  case llvm::CmpInst::ICMP_ULE:
  case llvm::CmpInst::ICMP_SLE:
    return "<operator>.lessEqualsThan";

  case llvm::CmpInst::FCMP_FALSE:
  case llvm::CmpInst::FCMP_ORD:
  case llvm::CmpInst::FCMP_UNO:
  case llvm::CmpInst::FCMP_TRUE:
  case llvm::CmpInst::BAD_FCMP_PREDICATE:
  case llvm::CmpInst::BAD_ICMP_PREDICATE:
    return std::string(instruction->getOpcodeName()) +
           llvm::CmpInst::getPredicateName(instruction->getPredicate()).str();
  }
}

std::string llvm2cpg::comparisonOperatorNameShort(const llvm::CmpInst *instruction) {
  switch (instruction->getPredicate()) {
  case llvm::CmpInst::FCMP_OEQ:
  case llvm::CmpInst::FCMP_UEQ:
  case llvm::CmpInst::ICMP_EQ:
    return "==";

  case llvm::CmpInst::FCMP_ONE:
  case llvm::CmpInst::FCMP_UNE:
  case llvm::CmpInst::ICMP_NE:
    return "!=";

  case llvm::CmpInst::FCMP_OGT:
  case llvm::CmpInst::FCMP_UGT:
  case llvm::CmpInst::ICMP_UGT:
  case llvm::CmpInst::ICMP_SGT:
    return ">";

  case llvm::CmpInst::FCMP_OLT:
  case llvm::CmpInst::FCMP_ULT:
  case llvm::CmpInst::ICMP_ULT:
  case llvm::CmpInst::ICMP_SLT:
    return "<";

  case llvm::CmpInst::ICMP_UGE:
  case llvm::CmpInst::FCMP_OGE:
  case llvm::CmpInst::FCMP_UGE:
  case llvm::CmpInst::ICMP_SGE:
    return ">=";

  case llvm::CmpInst::FCMP_OLE:
  case llvm::CmpInst::FCMP_ULE:
  case llvm::CmpInst::ICMP_ULE:
  case llvm::CmpInst::ICMP_SLE:
    return "<=";

  case llvm::CmpInst::FCMP_FALSE:
  case llvm::CmpInst::FCMP_ORD:
  case llvm::CmpInst::FCMP_UNO:
  case llvm::CmpInst::FCMP_TRUE:
  case llvm::CmpInst::BAD_FCMP_PREDICATE:
  case llvm::CmpInst::BAD_ICMP_PREDICATE:
    return std::string(instruction->getOpcodeName()) +
           llvm::CmpInst::getPredicateName(instruction->getPredicate()).str();
  }
}

std::string llvm2cpg::castOperatorName(const llvm::CastInst *instruction) {
  return "<operator>.cast";
}

std::string llvm2cpg::unaryOperatorName(const llvm::UnaryOperator *instruction) {
  return "<operator>.fneg";
}

std::string llvm2cpg::atomicOperatorName(const llvm::AtomicRMWInst *instruction) {
  std::string name("<operator>.atomic");
  switch (instruction->getOperation()) {
  case llvm::AtomicRMWInst::Xchg:
    return name + "Xchg";

  case llvm::AtomicRMWInst::Add:
  case llvm::AtomicRMWInst::FAdd:
    return name + "Addition";

  case llvm::AtomicRMWInst::Sub:
  case llvm::AtomicRMWInst::FSub:
    return name + "Subtraction";

  case llvm::AtomicRMWInst::And:
    return name + "And";
  case llvm::AtomicRMWInst::Nand:
    return name + "Nand";
  case llvm::AtomicRMWInst::Or:
    return name + "Or";
  case llvm::AtomicRMWInst::Xor:
    return name + "Xor";

  case llvm::AtomicRMWInst::UMax:
  case llvm::AtomicRMWInst::Max:
    return name + "Max";

  case llvm::AtomicRMWInst::UMin:
  case llvm::AtomicRMWInst::Min:
    return name + "Min";

  case llvm::AtomicRMWInst::BAD_BINOP:
    return name + "BAD_BINOP";
  }
}
