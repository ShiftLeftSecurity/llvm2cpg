#include "llvm2cpg/CPGWriter/CPGProtoWriter.h"

#include "CPGProtoAdapter.h"
#include <utility>

using namespace llvm2cpg;

CPGProtoWriter::CPGProtoWriter(std::string outputDir, bool debug)
    : outputDir(std::move(outputDir)), debug(debug) {}

void CPGProtoWriter::writeCpg(const llvm2cpg::CPG &cpg) {
  auto zipPath = outputDir + "/cpg.bin.zip";
  CPGProtoAdapter adapter(zipPath, debug);
  adapter.writeCpg(cpg);
}
