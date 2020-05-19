// RUN: %llvm2cpg --output=%t.cpg.bin.zip %p/fixtures/objc_methods.ll

// RUN: cd %ANALYZER_DIR
// RUN: %analyzer --script %s --params cpgFilePath=%t.cpg.bin.zip | %filecheck %s --match-full-lines


@main
def exec(cpgFilePath: String) = {
  workspace.reset
  importCpg(cpgFilePath)

  // Metaclass methods

  var rootClass = cpg.typeDecl.nameExact("RootClass$").head
  println(rootClass.start.method.size)
  // CHECK: 0
  println(rootClass.start.boundMethod.size)
  // CHECK: 1

  println(rootClass.start.boundMethod.nameExact("overridden").size)
  // CHECK: 1
  println(rootClass.start.boundMethod.nameExact("overridden").fullName.head)
  // CHECK: +[RootClass overridden]

  var child = cpg.typeDecl.nameExact("Child$").head
  println(child.start.method.size)
  // CHECK: 0
  println(child.start.boundMethod.size)
  // CHECK: 2

  println(child.start.boundMethod.nameExact("newChild").size)
  // CHECK: 1
  println(child.start.boundMethod.nameExact("newChild").fullName.head)
  // CHECK: +[Child newChild]

  println(child.start.boundMethod.nameExact("overridden").size)
  // CHECK: 1
  println(child.start.boundMethod.nameExact("overridden").fullName.head)
  // CHECK: +[Child overridden]

  // Metaclass Pointer methods

  rootClass = cpg.typeDecl.nameExact("RootClass$*").head
  println(rootClass.start.method.size)
  // CHECK: 0
  println(rootClass.start.boundMethod.size)
  // CHECK: 1

  println(rootClass.start.boundMethod.nameExact("overridden").size)
  // CHECK: 1
  println(rootClass.start.boundMethod.nameExact("overridden").fullName.head)
  // CHECK: +[RootClass overridden]

  child = cpg.typeDecl.nameExact("Child$*").head
  println(child.start.method.size)
  // CHECK: 0
  println(child.start.boundMethod.size)
  // CHECK: 2

  println(child.start.boundMethod.nameExact("newChild").size)
  // CHECK: 1
  println(child.start.boundMethod.nameExact("newChild").fullName.head)
  // CHECK: +[Child newChild]

  println(child.start.boundMethod.nameExact("overridden").size)
  // CHECK: 1
  println(child.start.boundMethod.nameExact("overridden").fullName.head)
  // CHECK: +[Child overridden]


  // Class Methods

  rootClass = cpg.typeDecl.nameExact("RootClass").head

  println(rootClass.start.method.size)
  // CHECK: 3
  println(rootClass.start.method.nameExact("inherited").size)
  // CHECK: 1
  println(rootClass.start.method.nameExact("inherited").fullName.head)
  // CHECK: -[RootClass inherited]

  println(rootClass.start.method.nameExact("overridden").size)
  // CHECK: 2
  println(rootClass.start.method.fullNameExact("+[RootClass overridden]").size)
  // CHECK: 1
  println(rootClass.start.method.fullNameExact("-[RootClass overridden]").size)
  // CHECK: 1

  println(rootClass.start.boundMethod.size)
  // CHECK: 2
  println(rootClass.start.boundMethod.nameExact("inherited").size)
  // CHECK: 1
  println(rootClass.start.boundMethod.nameExact("inherited").fullName.head)
  // CHECK: -[RootClass inherited]

  println(rootClass.start.boundMethod.nameExact("overridden").size)
  // CHECK: 1
  println(rootClass.start.boundMethod.nameExact("overridden").fullName.head)
  // CHECK: -[RootClass overridden]

  child = cpg.typeDecl.nameExact("Child").head

  println(child.start.method.size)
  // CHECK: 4

  println(child.start.method.nameExact("doSomething").size)
  // CHECK: 1
  println(child.start.method.nameExact("doSomething").fullName.head)
  // CHECK: -[Child doSomething]

  println(child.start.method.nameExact("newChild").size)
  // CHECK: 1
  println(child.start.method.nameExact("newChild").fullName.head)
  // CHECK: +[Child newChild]

  println(child.start.method.nameExact("overridden").size)
  // CHECK: 2
  println(child.start.method.fullNameExact("+[Child overridden]").size)
  // CHECK: 1
  println(child.start.method.fullNameExact("-[Child overridden]").size)
  // CHECK: 1

  println(child.start.boundMethod.size)
  // CHECK: 3

  println(child.start.boundMethod.nameExact("inherited").size)
  // CHECK: 1
  println(child.start.boundMethod.nameExact("inherited").fullName.head)
  // CHECK: -[RootClass inherited]

  println(child.start.boundMethod.nameExact("doSomething").size)
  // CHECK: 1
  println(child.start.boundMethod.nameExact("doSomething").fullName.head)
  // CHECK: -[Child doSomething]

  println(child.start.boundMethod.nameExact("overridden").size)
  // CHECK: 1
  println(child.start.boundMethod.nameExact("overridden").fullName.head)
  // CHECK: -[Child overridden]


  // Class pointer methods

  rootClass = cpg.typeDecl.nameExact("RootClass*").head

  println(rootClass.start.method.size)
  // CHECK: 0

  println(rootClass.start.boundMethod.size)
  // CHECK: 2
  println(rootClass.start.boundMethod.nameExact("inherited").size)
  // CHECK: 1
  println(rootClass.start.boundMethod.nameExact("inherited").fullName.head)
  // CHECK: -[RootClass inherited]

  println(rootClass.start.boundMethod.nameExact("overridden").size)
  // CHECK: 1
  println(rootClass.start.boundMethod.nameExact("overridden").fullName.head)
  // CHECK: -[RootClass overridden]

  child = cpg.typeDecl.nameExact("Child*").head

  println(child.start.method.size)
  // CHECK: 0

  println(child.start.boundMethod.size)
  // CHECK: 3

  println(child.start.boundMethod.nameExact("inherited").size)
  // CHECK: 1
  println(child.start.boundMethod.nameExact("inherited").fullName.head)
  // CHECK: -[RootClass inherited]

  println(child.start.boundMethod.nameExact("doSomething").size)
  // CHECK: 1
  println(child.start.boundMethod.nameExact("doSomething").fullName.head)
  // CHECK: -[Child doSomething]

  println(child.start.boundMethod.nameExact("overridden").size)
  // CHECK: 1
  println(child.start.boundMethod.nameExact("overridden").fullName.head)
  // CHECK: -[Child overridden]
}

// CHECK: script finished successfully
