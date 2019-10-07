#include <iostream>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/Path.h>
#include <llvm2cpg/CPG/BitcodeLoader.h>
#include <llvm2cpg/Transforms/Transforms.h>

llvm::cl::OptionCategory BitcodeTransformerCategory("bitcode-transformer");

llvm::cl::list<std::string> BitcodePaths(llvm::cl::Positional, llvm::cl::OneOrMore,
                                         llvm::cl::desc("Bitcode files"),
                                         llvm::cl::cat(BitcodeTransformerCategory));

llvm::cl::opt<std::string>
    OutputDirectory("output-dir", llvm::cl::Optional,
                    llvm::cl::desc("Where to store transformed bitcode (defaults to '.')"),
                    llvm::cl::cat(BitcodeTransformerCategory), llvm::cl::init("."));

static std::string getOutputFilename(const std::string &input) {
  std::string inputFilename = llvm::sys::path::stem(input);
  std::string extension = llvm::sys::path::extension(input);
  std::string outputDir = OutputDirectory.getValue();
  std::string outputFilename = outputDir.append(llvm::sys::path::get_separator().str())
                                   .append(inputFilename)
                                   .append(".opt")
                                   .append(extension);
  return outputFilename;
}

int main(int argc, char **argv) {
  llvm::cl::HideUnrelatedOptions(BitcodeTransformerCategory);
  llvm::cl::ParseCommandLineOptions(argc, argv);

  llvm::LLVMContext context;
  llvm2cpg::BitcodeLoader loader;
  llvm2cpg::Transforms transforms;

  for (size_t i = 0; i < BitcodePaths.size(); i++) {
    std::string input = BitcodePaths[i];
    std::cout << "Processing " << input << "\n";
    std::unique_ptr<llvm::Module> bitcode = loader.loadBitcode(input, context);
    transforms.transformBitcode(*bitcode);
    std::string outputFilename = getOutputFilename(input);

    std::error_code err;
    llvm::raw_fd_ostream output(outputFilename, err);
    if (err) {
      std::cerr << err.message() << "\n";
    }
    llvm::WriteBitcodeToFile(*bitcode, output);
    std::cout << "Saved transformed bitcode to " << outputFilename << "\n";
  }

  return 0;
}