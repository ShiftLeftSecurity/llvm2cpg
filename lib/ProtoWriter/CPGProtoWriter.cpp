#include "llvm2cpg/CPGWriter/CPGProtoWriter.h"

#include "CPGProtoAdapter.h"
#include "llvm2cpg/CPG/CPG.h"
#include "llvm2cpg/CPG/CPGFile.h"
#include "llvm2cpg/Logger/CPGLogger.h"
#include <utility>

using namespace llvm2cpg;

CPGProtoWriter::CPGProtoWriter(CPGLogger &logger, std::string output)
    : logger(logger), output(std::move(output)) {}

void CPGProtoWriter::writeCpg(const llvm2cpg::CPG &cpg) {
  const std::vector<CPGFile> &files = cpg.getFiles();
  if (files.empty()) {
    logger.uiWarning("No bitcode files found.");
    return;
  }
  logger.logInfo(std::string("Emitting CPG for ") + std::to_string(files.size()) + " file(s)");
  CPGProtoAdapter adapter(logger, output);
  adapter.writeCpg(cpg);
}
