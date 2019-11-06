#pragma once

#include <string>

namespace llvm2cpg {

class CPGLogger;
enum class FileType { Unsupported, Bitcode, Binary, LLVM_IR };
FileType getFileType(llvm2cpg::CPGLogger &logger, const std::string &path);

} // namespace llvm2cpg