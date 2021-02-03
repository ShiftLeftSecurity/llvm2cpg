// REQUIRES: JOERN
// RUN: %clang -c -emit-llvm -g -fno-discard-value-names %p/fixtures/use_after_free.c -o %t.bc
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
  println(sink.reachableByFlows(source).filter(p => p.elements.size > 1).p)
  // We expect the following
  //
  // _________________________________________________________________________________________________________________________
  // | tracked          | lineNumber| method| file                                                                            |
  // |========================================================================================================================|
  // | free(*buf1.addr) | 7         | main  | /opt/ShiftLeft/llvm2cpg/tests/end2end-tests/data-flow/fixtures/use_after_free.c |
  // | free(*buf1.addr) | 8         | main  | /opt/ShiftLeft/llvm2cpg/tests/end2end-tests/data-flow/fixtures/use_after_free.c |

  // CHECK: | free(*buf1.addr) | 7         | main  | {{.*}}use_after_free.c |
  // CHECK-NEXT: | free(*buf1.addr) | 8         | main  | {{.*}}use_after_free.c |
}

// CHECK: script finished successfully
