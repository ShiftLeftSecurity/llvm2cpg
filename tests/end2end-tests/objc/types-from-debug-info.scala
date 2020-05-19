// RUN: %llvm2cpg --output=%t.cpg.bin.zip %p/fixtures/objc_types_from_debug_info.ll

// RUN: cd %ANALYZER_DIR
// RUN: %analyzer --script %s --params cpgFilePath=%t.cpg.bin.zip | %filecheck %s --match-full-lines

@main
def exec(cpgFilePath: String) = {
  workspace.reset
  importCpg(cpgFilePath)

  println(cpg.call("bytes").argument.isCall.head.typeFullName)
  // CHECK: NSData*
}

// CHECK: script finished successfully