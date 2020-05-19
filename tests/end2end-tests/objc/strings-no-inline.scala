// RUN: %llvm2cpg --output=%t.cpg.bin.zip -inline-strings=false %p/fixtures/objc_strings.ll

// RUN: cd %ANALYZER_DIR
// RUN: %analyzer --script %s --params cpgFilePath=%t.cpg.bin.zip | %filecheck %s --match-full-lines

@main
def exec(cpgFilePath: String) = {
  workspace.reset
  importCpg(cpgFilePath)

  println(cpg.method("use").block.astChildren.order(1).isCall.argument.isCall.argument.isCall.argument.isCall.argument.code.head)
  // CHECK: Hello

  println(cpg.method("send").callOut.name("createObject").argument.isCall.argument.isIdentifier.code.head)
  // CHECK: OBJC_SELECTOR_REFERENCES_
}

// CHECK: script finished successfully