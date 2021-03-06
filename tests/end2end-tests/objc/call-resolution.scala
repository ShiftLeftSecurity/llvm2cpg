// REQUIRES: OCULAR
// RUN: %llvm2cpg --output=%t.cpg.bin.zip %p/fixtures/objc_call_00_definition.ll %p/fixtures/objc_call_00_usage.ll

// RUN: cd %ANALYZER_DIR
// RUN: %analyzer --script %s --params cpgFilePath=%t.cpg.bin.zip | %filecheck %s --match-full-lines

@main
def exec(cpgFilePath: String) = {
  workspace.reset
  importCpg(cpgFilePath)

  println(cpg.method.name("main").callOut.name("new").calledMethod.fullName.head)
  // CHECK: +[NSObject new]

  println(cpg.method.name("main").callOut.name("description").calledMethod.fullName.head)
  // CHECK: -[NSObject description]
}

// CHECK: script finished successfully
