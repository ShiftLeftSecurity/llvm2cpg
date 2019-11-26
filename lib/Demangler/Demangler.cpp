#include "llvm2cpg/Demangler/Demangler.h"
#include <cxxabi.h>
#include <regex>
#include <sstream>

using namespace llvm2cpg;

enum class ManglingType { Unknown, CXX, ObjC };

struct ManglingResult {
  ManglingType type;
  std::string result;
};

ManglingResult demangleString(const std::string &mangledName) {
  if (mangledName[0] == 1) {
    return ManglingResult{ .type = ManglingType::ObjC, .result = mangledName.substr(1) };
  }

  int status;
  char *buffer = abi::__cxa_demangle(mangledName.c_str(), nullptr, nullptr, &status);
  if (status == 0 && buffer != nullptr) {
    /// Success
    return ManglingResult{ .type = ManglingType::CXX, .result = std::string(buffer) };
  }

  return ManglingResult{ .type = ManglingType::Unknown, .result = mangledName };
}

std::string Demangler::extractFullName(const std::string &mangledName) {
  ManglingResult result = demangleString(mangledName);
  return result.result;
}

static std::string extractNameFromCXXName(const std::string &demangledName) {
  /// Starting from right to left
  auto begin = demangledName.rbegin();
  auto end = demangledName.rend();
  auto spacePosition = demangledName.rend();

  /// Skip everything until the last argument's paren
  while (begin != end) {
    if (*begin == ')') {
      break;
    }
    ++begin;
  }

  if (begin == end) {
    return demangledName;
  }

  /// Skip all the arguments (including anything that is nested within the parens)
  int parensCount = 0;
  while (begin != end) {
    if (*begin == ')') {
      parensCount++;
    } else if (*begin == '(') {
      parensCount--;
    }

    ++begin;

    if (parensCount == 0) {
      break;
    }
  }
  if (begin == end || parensCount != 0) {
    return demangledName;
  }

  /// The character after the opening arguments paren is the end of the name
  auto nameEnd = begin;

  /// Skip template parameters if any (including all the nested stuff)
  int angleBracketsCount = 0;
  while (begin != end) {
    if (*begin == '>') {
      angleBracketsCount++;
    } else if (*begin == '<') {
      angleBracketsCount--;
    }

    ++begin;
    if (angleBracketsCount == 0) {
      break;
    }
  }

  /// Skip anything until the first ::, which concludes the name search
  while (begin != end) {
    if (*begin == ':') {
      break;
    } else if (*begin == ' ') {
      spacePosition = begin;
    }

    ++begin;
  }

  /// In some case C++ demangles name into the form 'int foo(blah)', we need to skip the first token
  /// if it's not an 'operator'

  auto position = std::distance(begin, demangledName.rend());
  auto length = std::distance(nameEnd, demangledName.rend()) - position;
  std::string name = demangledName.substr(position, length);

  if (spacePosition != end) {
    auto space = name.find(' ');
    if (space != std::string::npos) {
      std::string prefix = name.substr(0, space + 1);
      if (prefix != "operator ") {
        name = name.substr(space + 1);
      }
    }
  }
  if (name.empty()) {
    return demangledName;
  }

  return name;
}

std::string Demangler::extractName(const std::string &mangledName) {
  ManglingResult result = demangleString(mangledName);
  switch (result.type) {
  case ManglingType::Unknown:
    return result.result;
  case ManglingType::CXX:
    return extractNameFromCXXName(result.result);
  case ManglingType::ObjC:
    return result.result;
  }
}
