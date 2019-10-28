#include "llvm2cpg/CPGWriter/CPGProtoWriter.h"

#include "CPGProtoAdapter.h"
#include "llvm2cpg/Logger/CPGLogger.h"
#include <utility>

using namespace llvm2cpg;

CPGProtoWriter::CPGProtoWriter(CPGLogger &logger, std::string outputDir, std::string outputName)
    : logger(logger), outputDir(std::move(outputDir)), outputName(std::move(outputName)) {}

void CPGProtoWriter::writeCpg(const llvm2cpg::CPG &cpg) {
  auto zipPath = outputDir + "/" + outputName;
  CPGProtoAdapter adapter(logger, zipPath);
  adapter.writeCpg(cpg);
}
