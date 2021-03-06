#include "IntrinsicsDemangler.h"
#include <llvm/IR/Function.h>
#include <llvm/IR/Intrinsics.h>

std::string llvm2cpg::intrinsicName(const llvm::Function *function) {
  assert(function->isIntrinsic());
  switch (function->getIntrinsicID()) {
  case llvm::Intrinsic::ceil:
    return "ceil";
  case llvm::Intrinsic::cos:
    return "cos";
  case llvm::Intrinsic::exp:
    return "exp";
  case llvm::Intrinsic::exp2:
    return "exp2";
  case llvm::Intrinsic::fabs:
    return "fabs";
  case llvm::Intrinsic::floor:
    return "floor";
  case llvm::Intrinsic::log:
    return "log";
  case llvm::Intrinsic::log10:
    return "log10";
  case llvm::Intrinsic::log2:
    return "log2";
  case llvm::Intrinsic::memcpy:
  case llvm::Intrinsic::memcpy_element_unordered_atomic:
    return "memcpy";
  case llvm::Intrinsic::memmove:
  case llvm::Intrinsic::memmove_element_unordered_atomic:
    return "memmove";
  case llvm::Intrinsic::memset:
  case llvm::Intrinsic::memset_element_unordered_atomic:
    return "memset";
  case llvm::Intrinsic::objc_autorelease:
    return "objc_autorelease";
  case llvm::Intrinsic::objc_autoreleasePoolPop:
    return "objc_autoreleasePoolPop";
  case llvm::Intrinsic::objc_autoreleasePoolPush:
    return "objc_autoreleasePoolPush";
  case llvm::Intrinsic::objc_autoreleaseReturnValue:
    return "objc_autoreleaseReturnValue";
  case llvm::Intrinsic::objc_clang_arc_use:
    return "objc_clang_arc_use";
  case llvm::Intrinsic::objc_copyWeak:
    return "objc_copyWeak";
  case llvm::Intrinsic::objc_destroyWeak:
    return "objc_destroyWeak";
  case llvm::Intrinsic::objc_initWeak:
    return "objc_initWeak";
  case llvm::Intrinsic::objc_loadWeak:
    return "objc_loadWeak";
  case llvm::Intrinsic::objc_loadWeakRetained:
    return "objc_loadWeakRetained";
  case llvm::Intrinsic::objc_moveWeak:
    return "objc_moveWeak";
  case llvm::Intrinsic::objc_release:
    return "objc_release";
  case llvm::Intrinsic::objc_retain:
    return "objc_retain";
  case llvm::Intrinsic::objc_retain_autorelease:
    return "objc_retain_autorelease";
  case llvm::Intrinsic::objc_retainAutorelease:
    return "objc_retainAutorelease";
  case llvm::Intrinsic::objc_retainAutoreleaseReturnValue:
    return "objc_retainAutoreleaseReturnValue";
  case llvm::Intrinsic::objc_retainAutoreleasedReturnValue:
    return "objc_retainAutoreleasedReturnValue";
  case llvm::Intrinsic::objc_retainBlock:
    return "objc_retainBlock";
  case llvm::Intrinsic::objc_retainedObject:
    return "objc_retainedObject";
  case llvm::Intrinsic::objc_storeStrong:
    return "objc_storeStrong";
  case llvm::Intrinsic::objc_storeWeak:
    return "objc_storeWeak";
  case llvm::Intrinsic::objc_sync_enter:
    return "objc_sync_enter";
  case llvm::Intrinsic::objc_sync_exit:
    return "objc_sync_exit";
  case llvm::Intrinsic::objc_unretainedObject:
    return "objc_unretainedObject";
  case llvm::Intrinsic::objc_unretainedPointer:
    return "objc_unretainedPointer";
  case llvm::Intrinsic::objc_unsafeClaimAutoreleasedReturnValue:
    return "objc_unsafeClaimAutoreleasedReturnValue";
  case llvm::Intrinsic::pow:
  case llvm::Intrinsic::powi:
    return "pow";
  case llvm::Intrinsic::round:
    return "round";
  case llvm::Intrinsic::sin:
    return "sin";
  case llvm::Intrinsic::sqrt:
    return "sqrt";
  case llvm::Intrinsic::vacopy:
    return "va_copy";
  case llvm::Intrinsic::vaend:
    return "va_end";
  case llvm::Intrinsic::vastart:
    return "va_start";
  default:
    return function->getName().str();
  }
}
