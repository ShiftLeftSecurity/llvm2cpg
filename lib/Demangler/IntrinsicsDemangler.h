#pragma once

#include <string>

namespace llvm {

class Function;

}

namespace llvm2cpg {

std::string intrinsicName(const llvm::Function *function);

}
