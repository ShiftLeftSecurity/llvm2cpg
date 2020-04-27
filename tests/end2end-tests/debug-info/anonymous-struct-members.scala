// RUN: %clang -c -emit-llvm -fno-discard-value-names -g %p/fixtures/debug_anonymous_struct_members.c -o %t.bc
// RUN: %llvm2cpg --output=%t.cpg.bin.zip %t.bc

// RUN: cd %OCULAR_DIR
// RUN: %ocular.sh --script %s --params cpgFilePath=%t.cpg.bin.zip | %filecheck %s --match-full-lines

@main
def exec(cpgFilePath: String) = {
  workspace.reset
  importCpg(cpgFilePath)

  println(cpg.typeDecl.name("Point").member.size)
  // CHECK: 3
  println(cpg.typeDecl.name("Point").member.name.toList.sorted.mkString("\n"))
  // CHECK: something
  // CHECK-NEXT: x
  // CHECK-NEXT: y

  println(cpg.typeDecl.name("PointPointer").member.size)
  // CHECK: 3
  println(cpg.typeDecl.name("PointPointer").member.name.toList.sorted.mkString("\n"))
  // CHECK: something
  // CHECK-NEXT: x
  // CHECK-NEXT: y

  val anonymousStruct = cpg.typeDecl.name("anon").head
  println(cpg.typeDecl.name("anon").member.size)
  // CHECK: 2
}

// CHECK: script finished successfully
