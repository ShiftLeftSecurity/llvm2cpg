// RUN: %llvm2cpg --output=%t.cpg.bin.zip %data_dir/llvm2cpg-samples/iGoat/x86_64/*.bc

// RUN: cd %OCULAR_DIR
// RUN: %ocular.sh -J-Xmx2048m --script %s --params cpgFilePath=%t.cpg.bin.zip | %filecheck %s --match-full-lines

@main
def exec(cpgFilePath: String) = {
  workspace.reset
  importCpg(cpgFilePath)

  val call = cpg.call.name("setObject:forKey:inCollection:")
  val callArgs = call.map(c => c.start.argument.order(4).code.head + " = " + c.start.argument.order(3).code.head).toList.sorted.mkString("\n")

  println(callArgs)
  // CHECK: YapKeyEmail = JohnDoe@yap.com
  // CHECK-NEXT: YapKeyPassword = TheUnknown
}

// CHECK: script finished successfully

