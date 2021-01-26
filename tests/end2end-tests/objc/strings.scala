// RUN: %llvm2cpg --output=%t.cpg.bin.zip %p/fixtures/objc_strings.ll

// RUN: cd %ANALYZER_DIR
// RUN: %analyzer --script %s --params cpgFilePath=%t.cpg.bin.zip 2>&1 | %filecheck %s --match-full-lines

@main
def exec(cpgFilePath: String) = {
  workspace.reset
  importCpg(cpgFilePath)

  println(cpg.method("use").block.astChildren.isCall.order(1).isCall.argument.isLiteral.code.head)
  // CHECK: Hello
  println(cpg.method("use").block.astChildren.isCall.order(2).isCall.argument.isLiteral.code.head)
  // CHECK: Hello
  println(cpg.method("use").block.astChildren.isCall.order(3).isCall.argument.isLiteral.code.head)
  // CHECK: world
  println(cpg.method("use").block.astChildren.order(4).isCall.argument.isCall.argument.isCall.argument.isLiteral.code.head)
  // CHECK: world

  println(cpg.method("send").callOut.name("createObject").argument.isLiteral.code.head)
  // CHECK: createObject
}

// CHECK: script finished successfully