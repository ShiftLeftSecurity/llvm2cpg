#pragma once

#include <llvm2cpg/CPGWriter/CPGWriter.h>
#include <string>

namespace llvm2cpg {

class CPGProtoWriter : public CPGWriter {
public:
  explicit CPGProtoWriter(std::string outputDir, bool debug = false);
  void writeCpg(const CPG &cpg) override;

private:
  std::string outputDir;
  bool debug;
};

} // namespace llvm2cpg