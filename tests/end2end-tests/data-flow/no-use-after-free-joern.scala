// REQUIRES: JOERN
// RUN: %clang -c -emit-llvm -g -fno-discard-value-names %p/fixtures/no_use_after_free.c -o %t.bc
// RUN: %llvm2cpg --output=%t.cpg.bin.zip %t.bc

// RUN: cd %ANALYZER_DIR
// RUN: %analyzer --script %s --params cpgFilePath=%t.cpg.bin.zip 2>&1 | %filecheck %s --match-full-lines --dump-input=fail

@main
def exec(cpgFilePath: String) = {
  workspace.reset
  importCpg(cpgFilePath)
  run.ossdataflow

  def source = cpg.call("free").argument
  def sink = cpg.call.argument
  val flows = sink.reachableByFlows(source).filter(p => p.elements.size > 1).p
  println("anchor")
  println(flows)
  // CHECK: anchor
  // CHECK-NEXT: List()
  // CHECK-NEXT: script finished successfully
}
