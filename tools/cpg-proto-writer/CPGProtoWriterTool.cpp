#include "FileType.h"
#include <llvm/Support/CommandLine.h>
#include <llvm2cpg/CPG/BitcodeLoader.h>
#include <llvm2cpg/CPG/CPG.h>
#include <llvm2cpg/CPGWriter/CPGProtoWriter.h>
#include <llvm2cpg/Logger/CPGLogger.h>
#include <string>

llvm::cl::OptionCategory CPGProtoWriterCategory("cpg-proto-writer");

llvm::cl::list<std::string> BitcodePaths(llvm::cl::Positional, llvm::cl::OneOrMore,
                                         llvm::cl::desc("Bitcode files"),
                                         llvm::cl::cat(CPGProtoWriterCategory));

llvm::cl::opt<std::string>
    OutputDirectory("output-dir", llvm::cl::Optional,
                    llvm::cl::desc("Where to store cpg.bin.zip (defaults to '.')"),
                    llvm::cl::cat(CPGProtoWriterCategory), llvm::cl::init("."));

llvm::cl::opt<std::string> OutputName("output-name", llvm::cl::Optional,
                                      llvm::cl::desc("Output filename (defaults to 'cpg.bin.zip')"),
                                      llvm::cl::cat(CPGProtoWriterCategory),
                                      llvm::cl::init("cpg.bin.zip"));

int main(int argc, char **argv) {
  llvm::cl::HideUnrelatedOptions(CPGProtoWriterCategory);
  llvm::cl::ParseCommandLineOptions(argc, argv);

  llvm::LLVMContext context;
  llvm2cpg::CPGLogger logger;
  llvm2cpg::BitcodeLoader loader(logger);
  llvm2cpg::CPG cpg;
  std::vector<std::unique_ptr<llvm::Module>> bitcode;
  for (size_t i = 0; i < BitcodePaths.size(); i++) {
    std::string path = BitcodePaths[i];
    llvm2cpg::FileType type = getFileType(logger, path);
    switch (type) {
    case llvm2cpg::FileType::Unsupported: {
      logger.warning(std::string("Skipping unsupported file ") + path);
    } break;
    case llvm2cpg::FileType::Bitcode: {
      logger.info(std::string("Parsing bitcode file ") + path);
      std::unique_ptr<llvm::Module> module = loader.loadBitcode(path, context);
      if (module) {
        bitcode.push_back(std::move(module));
        cpg.addBitcode(bitcode.back().get());
      }
    } break;
    case llvm2cpg::FileType::Binary: {
      logger.info(std::string("Attempting to extract bitcode from ") + path);
      for (std::unique_ptr<llvm::Module> &module : loader.extractBitcode(path, context)) {
        bitcode.push_back(std::move(module));
        cpg.addBitcode(bitcode.back().get());
      }
    } break;
    case llvm2cpg::FileType::LLVM_IR: {
      logger.info(std::string("Parsing IR file ") + path);
      std::unique_ptr<llvm::Module> module = loader.loadIR(path, context);
      if (module) {
        bitcode.push_back(std::move(module));
        cpg.addBitcode(bitcode.back().get());
      }
    } break;
    }
  }

  llvm2cpg::CPGProtoWriter writer(logger, OutputDirectory.getValue(), OutputName.getValue());
  writer.writeCpg(cpg);

  return 0;
}
