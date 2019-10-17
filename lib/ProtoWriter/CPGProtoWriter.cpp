#include "llvm2cpg/CPGWriter/CPGProtoWriter.h"

#include "CPGProtoAdapter.h"
#include <utility>

using namespace llvm2cpg;

CPGProtoWriter::CPGProtoWriter(std::string outputDir, std::string outputName, bool debug)
    : outputDir(std::move(outputDir)), outputName(std::move(outputName)), debug(debug) {}

void CPGProtoWriter::writeCpg(const llvm2cpg::CPG &cpg) {
  auto zipPath = outputDir + "/" + outputName;
  CPGProtoAdapter adapter(zipPath, debug);
  adapter.writeCpg(cpg);
}
