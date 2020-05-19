// RUN: %llvm2cpg --output=%t.cpg.bin.zip %p/fixtures/objc_classes.ll

// RUN: cd %ANALYZER_DIR
// RUN: %analyzer --script %s --params cpgFilePath=%t.cpg.bin.zip | %filecheck %s --match-full-lines

@main
def exec(cpgFilePath: String) = {
  workspace.reset
  importCpg(cpgFilePath)

  // Normal classes

  println(cpg.typeDecl.nameExact("RootClass").size)
  // CHECK: 1
  println(cpg.typeDecl.nameExact("Child").size)
  // CHECK: 1

  println(cpg.typeDecl.nameExact("RootClass").baseTypeDecl.size)
  // CHECK: 0
  println(cpg.typeDecl.nameExact("Child").baseTypeDecl.size)
  // CHECK: 1
  println(cpg.typeDecl.nameExact("Child").baseTypeDecl.nameExact("RootClass").size)
  // CHECK: 1

  println(cpg.typeDecl.nameExact("RootClass").derivedTypeDecl.size)
  // CHECK: 1
  println(cpg.typeDecl.nameExact("RootClass").derivedTypeDecl.nameExact("Child").size)
  // CHECK: 1
  println(cpg.typeDecl.nameExact("Child").derivedTypeDecl.size)
  // CHECK: 0

  // Meta classes

  println(cpg.typeDecl.nameExact("RootClass$").size)
  // CHECK: 1
  println(cpg.typeDecl.nameExact("Child$").size)
  // CHECK: 1

  println(cpg.typeDecl.nameExact("RootClass$").baseTypeDecl.size)
  // CHECK: 0
  println(cpg.typeDecl.nameExact("Child$").baseTypeDecl.size)
  // CHECK: 1
  println(cpg.typeDecl.nameExact("Child$").baseTypeDecl.nameExact("RootClass$").size)
  // CHECK: 1

  println(cpg.typeDecl.nameExact("RootClass$").derivedTypeDecl.size)
  // CHECK: 1
  println(cpg.typeDecl.nameExact("RootClass$").derivedTypeDecl.nameExact("Child$").size)
  // CHECK: 1
  println(cpg.typeDecl.nameExact("Child$").derivedTypeDecl.size)
  // CHECK: 0

  val method =  cpg.method.nameExact("useChild").head
  println(method.start.callOut.name("newChild").argument.size)
  // CHECK: 2

  println(method.start.callOut.name("doSomething").calledMethod.fullName.head)
  // CHECK: -[Child doSomething]
  println(method.start.callOut.name("doSomething").argument.size)
  // CHECK: 2
  println(method.start.callOut.name("doSomething").argument.isLiteral.code.head)
  // CHECK: doSomething

  println(method.start.callOut.name("newChild").argument.size)
  // CHECK: 2
  println(method.start.callOut.name("newChild").argument.isLiteral.code.head)
  // CHECK: newChild

  // The latest ocular release (0.3.110) doesn't support this case yet
  // println(method.start.callOut.name("newChild").calledMethod.fullName.head)
  // CHECKX: +[Child newChild]
}

// CHECK: script finished successfully
