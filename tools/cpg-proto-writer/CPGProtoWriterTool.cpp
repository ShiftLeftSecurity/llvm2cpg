#include <iostream>
#include <llvm/Support/CommandLine.h>
#include <llvm2cpg/CPG/BitcodeLoader.h>
#include <llvm2cpg/CPG/CPG.h>
#include <llvm2cpg/CPGWriter/CPGProtoWriter.h>
#include <string>

llvm::cl::OptionCategory CPGProtoWriterCategory("cpg-proto-writer");

llvm::cl::list<std::string> BitcodePaths(llvm::cl::Positional, llvm::cl::OneOrMore,
                                         llvm::cl::desc("Bitcode files"),
                                         llvm::cl::cat(CPGProtoWriterCategory));

llvm::cl::opt<std::string>
    OutputDirectory("output-dir", llvm::cl::Optional,
                    llvm::cl::desc("Where to store cpg.bin.zip (defaults to '.')"),
                    llvm::cl::cat(CPGProtoWriterCategory), llvm::cl::init("."));

int main(int argc, char **argv) {
  llvm::cl::HideUnrelatedOptions(CPGProtoWriterCategory);
  llvm::cl::ParseCommandLineOptions(argc, argv);

  llvm::LLVMContext context;
  llvm2cpg::BitcodeLoader loader;
  llvm2cpg::CPG cpg;
  std::vector<std::unique_ptr<llvm::Module>> bitcode;
  for (size_t i = 0; i < BitcodePaths.size(); i++) {
    auto path = BitcodePaths[i];
    std::cout << "Processing " << path << "\n";
    auto bc = loader.loadBitcode(path, context);
    bitcode.push_back(std::move(bc));
    cpg.addBitcode(bitcode.back().get());
  }

  llvm2cpg::CPGProtoWriter writer(OutputDirectory.getValue());
  writer.writeCpg(cpg);

  return 0;
}
