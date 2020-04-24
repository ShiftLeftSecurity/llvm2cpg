#pragma once

#include <string>

namespace llvm2cpg {

enum class ManglingType { Unknown, CXX, ObjC };

struct ManglingResult {
  ManglingType type;
  std::string result;
};

ManglingResult demangleString(const std::string &mangledName);

class Demangler {
public:
  std::string extractFullName(const std::string &mangledName);
  std::string extractName(const std::string &mangledName);

private:
};

} // namespace llvm2cpg