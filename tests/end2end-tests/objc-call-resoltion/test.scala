/*
RUN: %llvm2cpg --output=%t.cpg.bin.zip %p/objc_call_00_definition.ll %p/objc_call_00_usage.ll

RUN: cd %OCULAR_DIR
RUN: %ocular.sh --script %s --params cpgFilePath=%t.cpg.bin.zip | %FileCheck %s --match-full-lines
*/

@main
def exec(cpgFilePath: String) = {
  workspace.reset
  importCpg(cpgFilePath)
  addOverlay("dataflow")
  addOverlay("semanticcpg")

  println(cpg.method.name("main").callOut.name("new").calledMethod.fullName.head)
  // CHECK: +[NSObject new]

  println(cpg.method.name("main").callOut.name("description").calledMethod.fullName.head)
  // CHECK: -[NSObject description]
}

// CHECK: script finished successfully
