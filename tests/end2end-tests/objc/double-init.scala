// REQUIRES: OCULAR
// RUN: %llvm2cpg --output=%t.cpg.bin.zip %p/fixtures/double-init.ll

// RUN: cd %ANALYZER_DIR
// RUN: %analyzer --script %s --params cpgFilePath=%t.cpg.bin.zip | %filecheck %s --match-full-lines --strict-whitespace


@main
def exec(cpgFilePath: String) = {
  workspace.reset
  importCpg(cpgFilePath)

  def source = cpg.call.name("initWithBytes:length:")
  def sink = cpg.call.name("initWithBytes:length:").receiver
  println(sink.reachableBy(source).flows.p.mkString("\n"))

  //  It is expected to see a flow similar to the following one:
  // __________________________________________________________________
  // | tracked              | lineNumber| method               | file  |
  // |=================================================================|
  // | initWithBytes:length:| 21        | main                 | main.m|
  // | p2                   | N/A       | <operator>.assignment|       |
  // | p1                   | N/A       | <operator>.assignment|       |
  // | tmp4                 | 21        | main                 | main.m|
  // | tmp4                 | 21        | main                 | main.m|
  // | p2                   | N/A       | <operator>.cast      |       |
  // | ret                  | N/A       | <operator>.cast      |       |
  // | (FooClass*)tmp4      | 21        | main                 | main.m|
  // | p2                   | N/A       | <operator>.assignment|       |
  // | p1                   | N/A       | <operator>.assignment|       |
  // | *obj.addr            | 21        | main                 | main.m|
  // | *obj.addr            | 22        | main                 | main.m|
  //
  // We check only a few lines to ensure we've got the right output

  // CHECK: | tracked              | lineNumber| method               | file  |
  // CHECK: | initWithBytes:length:| 21        | main                 | main.m|
  // CHECK: | *obj.addr            | 22        | main                 | main.m|
}

// CHECK:script finished successfully
