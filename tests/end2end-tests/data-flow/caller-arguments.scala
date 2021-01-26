// RUN: %clang -c -emit-llvm -fno-discard-value-names %p/fixtures/dataflow_caller_arguments.c -o %t.bc
// RUN: %llvm2cpg --output=%t.cpg.bin.zip %t.bc

// RUN: cd %ANALYZER_DIR
// RUN: %analyzer --script %s --params cpgFilePath=%t.cpg.bin.zip 2>&1 | %filecheck %s --match-full-lines

@main
def exec(cpgFilePath: String) = {
  workspace.reset
  importCpg(cpgFilePath)

  println(cpg.method.name("foo").parameter.argument.isLiteral.code.head)
  // CHECK: 42
}

// CHECK: script finished successfully
