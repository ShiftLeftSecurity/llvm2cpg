#pragma once

namespace llvm2cpg {

class CPG;

class CPGWriter {
public:
  virtual void writeCpg(const CPG &cpg) = 0;
  virtual ~CPGWriter() = default;

private:
};

} // namespace llvm2cpg
