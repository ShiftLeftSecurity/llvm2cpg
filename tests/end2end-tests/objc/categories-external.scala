// RUN: %llvm2cpg --output=%t.cpg.bin.zip %p/fixtures/objc_categories_external.ll

// RUN: cd %OCULAR_DIR
// RUN: %ocular.sh --script %s --params cpgFilePath=%t.cpg.bin.zip | %filecheck %s --match-full-lines

@main
def exec(cpgFilePath: String) = {
  workspace.reset
  importCpg(cpgFilePath)

  println(cpg.metaData.head.policyDirectories.toList.sorted.mkString("\n"))
  // CHECK: C
  // CHECK-NEXT: OBJECTIVEC

  // Class Type Decl

  println(cpg.typeDecl.nameExact("RootClass").size)
  // CHECK: 1
  println(cpg.typeDecl.nameExact("RootClass").baseTypeDecl.size)
  // CHECK: 0
  println(cpg.typeDecl.nameExact("RootClass").derivedTypeDecl.size)
  // CHECK: 0

  var rootClass = cpg.typeDecl.nameExact("RootClass").head
  println(rootClass.start.method.size)
  // CHECK: 0
  println(rootClass.start.boundMethod.size)
  // CHECK: 1
  println(rootClass.isExternal)
  // CHECK: true

  println(rootClass.start.boundMethod.nameExact("doSomething").fullName.head)
  // CHECK: -[RootClass(SomeCategory) doSomething]

  // Metaclass Type Decl
  println(cpg.typeDecl.nameExact("RootClass$").size)
  // CHECK: 1
  println(cpg.typeDecl.nameExact("RootClass$").baseTypeDecl.size)
  // CHECK: 0
  println(cpg.typeDecl.nameExact("RootClass$").derivedTypeDecl.size)
  // CHECK: 0

  rootClass = cpg.typeDecl.nameExact("RootClass$").head
  println(rootClass.start.method.size)
  // CHECK: 0
  println(rootClass.start.boundMethod.size)
  // CHECK: 1
  println(rootClass.isExternal)
  // CHECK: true

  println(rootClass.start.boundMethod.nameExact("doSomethingElse").fullName.head)
  // CHECK: +[RootClass(SomeCategory) doSomethingElse]

  // Category Type Decl
  println(cpg.typeDecl.nameExact("RootClass(SomeCategory)").size)
  // CHECK: 1
  println(cpg.typeDecl.nameExact("RootClass(SomeCategory)").baseTypeDecl.size)
  // CHECK: 0
  println(cpg.typeDecl.nameExact("RootClass(SomeCategory)").derivedTypeDecl.size)
  // CHECK: 0

  val category = cpg.typeDecl.nameExact("RootClass(SomeCategory)").head
  println(category.start.method.size)
  // CHECK: 2
  println(category.start.boundMethod.size)
  // CHECK: 0

  println(category.start.method.nameExact("doSomething").fullName.head)
  // CHECK: -[RootClass(SomeCategory) doSomething]
  println(category.start.method.nameExact("doSomethingElse").fullName.head)
  // CHECK: +[RootClass(SomeCategory) doSomethingElse]
}

// CHECK: script finished successfully