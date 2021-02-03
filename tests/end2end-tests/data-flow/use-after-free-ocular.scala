// REQUIRES: OCULAR
// RUN: %clang -c -emit-llvm -g -fno-discard-value-names %p/fixtures/use_after_free.c -o %t.bc
// RUN: %llvm2cpg --output=%t.cpg.bin.zip %t.bc

// RUN: cd %ANALYZER_DIR
// RUN: %analyzer --script %s --params cpgFilePath=%t.cpg.bin.zip 2>&1 | %filecheck %s --match-full-lines --dump-input=fail

@main
def exec(cpgFilePath: String) = {
  workspace.reset
  importCpg(cpgFilePath)

  def source = cpg.call("free").argument
  def sink = cpg.call.argument
  println(sink.reachableBy(source).flows.filter(p => p.points.size > 1).p)
  // We expect the following
  //  ______________________________________________________________________________________________________________
  // | tracked   | lineNumber| method| file                                                                        |
  // |=============================================================================================================|
  // | *buf1.addr| 7         | main  | <truncated>/llvm2cpg/tests/end2end-tests/data-flow/fixtures/use_after_free.c|
  // | *buf1.addr| 8         | main  | <truncated>/llvm2cpg/tests/end2end-tests/data-flow/fixtures/use_after_free.c|
  // CHECK: | *buf1.addr| 7         | main  | {{.*}}use_after_free.c|
  // CHECK-NEXT: | *buf1.addr| 8         | main  | {{.*}}use_after_free.c|
}

// CHECK: script finished successfully
