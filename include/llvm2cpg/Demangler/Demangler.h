#pragma once

#include <string>

namespace llvm2cpg {

class Demangler {
public:
  std::string extractFullName(const std::string& mangledName);
  std::string extractName(const std::string& mangledName);

private:
};

}