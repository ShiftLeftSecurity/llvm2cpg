#include "llvm2cpg/CPG/Version.h"

#include <llvm/Support/raw_ostream.h>

namespace llvm2cpg {

const char *llvm2cpgVersionString() {
  return "@PROJECT_VERSION@";
}

const char *llvm2cpgCommitString() {
  return "@GIT_COMMIT@";
}

const char *llvm2cpgBuildDateString() {
  return "@BUILD_DATE@";
}

const char *llvmVersionString() {
  return "@LLVM_VERSION@";
}

void printVersionInformationStream(llvm::raw_ostream &out) {
  out << "Version: " << llvm2cpgVersionString() << "\n";
  out << "Commit: " << llvm2cpgCommitString() << "\n";
  out << "Date: " << llvm2cpgBuildDateString() << "\n";
  out << "LLVM: " << llvmVersionString() << "\n";
}
} // namespace llvm2cpg