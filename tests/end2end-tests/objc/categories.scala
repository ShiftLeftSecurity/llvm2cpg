// RUN: %llvm2cpg --output=%t.cpg.bin.zip %p/fixtures/objc_categories.ll

// RUN: cd %ANALYZER_DIR
// RUN: %analyzer --script %s --params cpgFilePath=%t.cpg.bin.zip | %filecheck %s --match-full-lines

@main
def exec(cpgFilePath: String) = {
  workspace.reset
  importCpg(cpgFilePath)

  // Class Type Decl

  println(cpg.typeDecl.nameExact("RootClass").size)
  // CHECK: 1
  println(cpg.typeDecl.nameExact("RootClass").baseTypeDecl.size)
  // CHECK: 0
  println(cpg.typeDecl.nameExact("RootClass").derivedTypeDecl.size)
  // CHECK: 0

  var rootClass = cpg.typeDecl.nameExact("RootClass").head
  println(rootClass.start.method.size)
  // CHECK: 2
  println(rootClass.start.boundMethod.size)
  // CHECK: 2

  println(rootClass.start.boundMethod.nameExact("init").size)
  // CHECK: 1
  println(rootClass.start.boundMethod.nameExact("init").fullName.head)
  // CHECK: -[RootClass init]

  println(rootClass.start.boundMethod.nameExact("doSomething").size)
  // CHECK: 1
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
  // CHECK: 2

  println(rootClass.start.boundMethod.nameExact("alloc").fullName.head)
  // CHECK: +[RootClass alloc]

  println(rootClass.start.boundMethod.nameExact("doSomethingElse").fullName.head)
  // CHECK: +[RootClass(SomeCategory) doSomethingElse]
}

// CHECK: script finished successfully