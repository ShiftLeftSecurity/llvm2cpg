#include <iostream>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/Path.h>
#include <llvm2cpg/CPG/BitcodeLoader.h>
#include <llvm2cpg/Logger/CPGLogger.h>
#include <llvm2cpg/Transforms/Transforms.h>

llvm::cl::OptionCategory BitcodeTransformerCategory("bitcode-transformer");

llvm::cl::list<std::string> BitcodePaths(llvm::cl::Positional, llvm::cl::OneOrMore,
                                         llvm::cl::desc("Bitcode files"),
                                         llvm::cl::cat(BitcodeTransformerCategory));

llvm::cl::opt<std::string>
    OutputDirectory("output-dir", llvm::cl::Optional,
                    llvm::cl::desc("Where to store transformed bitcode (defaults to '.')"),
                    llvm::cl::cat(BitcodeTransformerCategory), llvm::cl::init("."));

llvm::cl::opt<bool>
    APInliner("inline", llvm::cl::Optional,
              llvm::cl::desc("Enable inlining of access paths (loads, pointer arithmetic)"),
              llvm::cl::cat(BitcodeTransformerCategory), llvm::cl::init(true));

llvm::cl::opt<bool> InlineStrings("inline-strings", llvm::cl::Optional,
                                  llvm::cl::desc("Enable global strings inlining"),
                                  llvm::cl::cat(BitcodeTransformerCategory), llvm::cl::init(true));

llvm::cl::opt<bool> SimplifyBC("simplify", llvm::cl::Optional,
                               llvm::cl::desc("Enable simplification of bitcode"),
                               llvm::cl::cat(BitcodeTransformerCategory), llvm::cl::init(false));

static std::string getOutputFilename(const std::string &input) {
  std::string inputFilename = llvm::sys::path::stem(input).str();
  std::string extension = llvm::sys::path::extension(input).str();
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
  llvm2cpg::CPGLogger log(false);
  llvm2cpg::BitcodeLoader loader(log);
  llvm2cpg::Transforms transforms(
      log, APInliner.getValue(), SimplifyBC.getValue(), InlineStrings.getValue());

  for (size_t i = 0; i < BitcodePaths.size(); i++) {
    std::string input = BitcodePaths[i];
    std::cout << "Processing " << input << "\n";
    std::unique_ptr<llvm::Module> bitcode = loader.loadBitcode(input);
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