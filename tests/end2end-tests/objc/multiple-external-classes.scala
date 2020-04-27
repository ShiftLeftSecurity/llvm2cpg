// RUN: %llvm2cpg --output=%t.cpg.bin.zip %p/fixtures/objc_external_class.ll %p/fixtures/objc_external_class_2.ll

// RUN: cd %OCULAR_DIR
// RUN: %ocular.sh --script %s --params cpgFilePath=%t.cpg.bin.zip | %filecheck %s --match-full-lines

@main
def exec(cpgFilePath: String) = {
  workspace.reset
  importCpg(cpgFilePath)

  println(cpg.typeDecl.nameExact("RootClass").size)
  // CHECK: 1

  val root = cpg.typeDecl.nameExact("RootClass").head
  println(root.isExternal)
  // CHECK: true
  println(root.start.method.size)
  // CHECK: 0
  println(root.start.boundMethod.size)
  // CHECK: 0

  println(cpg.typeDecl.nameExact("RootClass").baseTypeDecl.size)
  // CHECK: 0

  println(cpg.typeDecl.nameExact("Child").baseTypeDecl.size)
  // CHECK: 1
  println(cpg.typeDecl.nameExact("Child").baseTypeDecl.name.head)
  // CHECK: RootClass
  println(cpg.typeDecl.nameExact("Child2").baseTypeDecl.size)
  // CHECK: 1
  println(cpg.typeDecl.nameExact("Child2").baseTypeDecl.name.head)
  // CHECK: RootClass

  println(cpg.typeDecl.nameExact("RootClass").derivedTypeDecl.size)
  // CHECK: 2
  println(cpg.typeDecl.nameExact("RootClass").derivedTypeDecl.nameExact("Child").size)
  // CHECK: 1
  println(cpg.typeDecl.nameExact("Child").derivedTypeDecl.size)
  // CHECK: 0
  println(cpg.typeDecl.nameExact("RootClass").derivedTypeDecl.nameExact("Child2").size)
  // CHECK: 1
  println(cpg.typeDecl.nameExact("Child2").derivedTypeDecl.size)
  // CHECK: 0
}

// CHECK: script finished successfully