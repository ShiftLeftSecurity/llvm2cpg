// RUN: %clang -c -emit-llvm -fno-discard-value-names %p/fixtures/dataflow_caller.c -o %t.bc
// RUN: %llvm2cpg --output=%t.cpg.bin.zip %t.bc

// RUN: cd %ANALYZER_DIR
// RUN: %analyzer --script %s --params cpgFilePath=%t.cpg.bin.zip | %filecheck %s --match-full-lines

@main
def exec(cpgFilePath: String) = {
  workspace.reset
  importCpg(cpgFilePath)

  val callee = cpg.method.name("foo").head
  val caller = cpg.method.name("bar").head

  println(callee.start.caller.name.head)
  // CHECK: bar

  println(caller.start.callee.name.head)
  // CHECK: foo

  val callNode = caller.start.block.astChildren.isCall.head
  println(callNode.start.calledMethod.name.head)
  // CHECK: foo
}

// CHECK: script finished successfully
