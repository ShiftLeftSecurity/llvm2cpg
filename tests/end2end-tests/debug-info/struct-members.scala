// RUN: %clang -c -emit-llvm -g %p/fixtures/debug_struct_members.c -o %t.bc
// RUN: %llvm2cpg --output=%t.cpg.bin.zip -inline=false %t.bc

// RUN: cd %OCULAR_DIR
// RUN: %ocular.sh --script %s --params cpgFilePath=%t.cpg.bin.zip | %filecheck %s --match-full-lines

@main
def exec(cpgFilePath: String) = {
  workspace.reset
  importCpg(cpgFilePath)

  println(cpg.typeDecl.nameExact("Point").member.size)
  // CHECK: 2
  println(cpg.typeDecl.nameExact("Point").member.name.toList.sorted.mkString("\n"))
  // CHECK: x
  // CHECK-NEXT: y

  println(cpg.typeDecl.nameExact("PointPointer").member.size)
  // CHECK: 2
  println(cpg.typeDecl.nameExact("PointPointer").member.name.toList.sorted.mkString("\n"))
  // CHECK: x
  // CHECK-NEXT: y

  println(cpg.typeDecl.nameExact("PointTypedef").member.size)
  // CHECK: 2
  println(cpg.typeDecl.nameExact("PointTypedef").member.name.toList.sorted.mkString("\n"))
  // CHECK: x
  // CHECK-NEXT: y


  val gep1 = cpg.method.name("usePoint").block.astChildren.isCall.order(3).astChildren.isCall.head
  val gep1Reference = gep1.start.astChildren.isFieldIdentifier.head
  println(gep1.name)
  // CHECK: <operator>.getElementPtr
  println(gep1Reference.code)
  // CHECK: x
  println(gep1Reference.canonicalName)
  // CHECK: x

  val gep2 = cpg.method.name("usePoint").block.astChildren.isCall.order(5).astChildren.isCall.head
  val gep2Reference = gep2.start.astChildren.isFieldIdentifier.head

  println(gep2.name)
  // CHECK: <operator>.getElementPtr
  println(gep2Reference.code)
  // CHECK: y
  println(gep2Reference.canonicalName)
  // CHECK: y
}

// CHECK: script finished successfully
