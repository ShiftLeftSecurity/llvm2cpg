#pragma once

namespace llvm {
class raw_ostream;
}

namespace llvm2cpg {

const char *llvm2cpgVersionString();
const char *llvm2cpgCommitString();
const char *llvm2cpgBuildDateString();

const char *llvmVersionString();
void printVersionInformationStream(llvm::raw_ostream &out);

} // namespace llvm2cpg
