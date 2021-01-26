// RUN: %clang -c -emit-llvm -fno-discard-value-names %p/fixtures/call_cast.c -o %t.bc
// RUN: %llvm2cpg --output=%t.cpg.bin.zip %t.bc

// RUN: cd %ANALYZER_DIR
// RUN: %analyzer --script %s --params cpgFilePath=%t.cpg.bin.zip 2>&1 | %filecheck %s --match-full-lines

@main
def exec(cpgFilePath: String) = {
  workspace.reset
  importCpg(cpgFilePath)

  println(cpg.method.nameExact("ecall").size)
  // CHECK: 1
  println(cpg.method.nameExact("use").size)
  // CHECK: 1

  val use = cpg.method.nameExact("use").head

  println(use.start.callOut.nameExact("ecall").calledMethod.name.head)
  // CHECK: ecall
}

// CHECK: script finished successfully
