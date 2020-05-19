// RUN: %clang -c -emit-llvm -fno-discard-value-names -g %p/fixtures/debug_nested_struct_members.c -o %t.bc
// RUN: %llvm2cpg --output=%t.cpg.bin.zip %t.bc

// RUN: cd %ANALYZER_DIR
// RUN: %analyzer --script %s --params cpgFilePath=%t.cpg.bin.zip | %filecheck %s --match-full-lines

@main
def exec(cpgFilePath: String) = {
  workspace.reset
  importCpg(cpgFilePath)

  val person = cpg.typeDecl.name("Person").head
  println(person.start.member.size)
  // CHECK: 3
  println(person.start.member.name.toList.sorted.mkString("\n"))
  // CHECK: age
  // CHECK-NEXT: hobbies
  // CHECK-NEXT: name

  val hobby = cpg.typeDecl.name("Hobby").head
  println(hobby.start.member.size)
  // CHECK: 2
  println(hobby.start.member.name.toList.sorted.mkString("\n"))
  // CHECK: hobbyName
  // CHECK-NEXT: length

  val name = cpg.typeDecl.name("Name").head
  println(name.start.member.size)
  // CHECK: 2
  println(name.start.member.name.toList.sorted.mkString("\n"))
  // CHECK: buffer
  // CHECK-NEXT: length
}

// CHECK: script finished successfully
