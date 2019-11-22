package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._

class ObjC_ClassesTest extends CPGMatcher {
  private val cpg = CpgLoader.load(TestCpgPaths.ObjC_ClassesTestCPG)

  "types" in {
    validateTypes(cpg, Set("ANY",
      "i8* (i8*, i8*)*",
      "Child**",
      "i8* (i8*, i8*)",
      "RootClass**",
      "void",
      "i8* (RootClass*, i8*)",
      "void (i8*, i8*)*",
      "struct._class_t**",
      "RootClass*",
      "i8**",
      "Child*",
      "struct._class_t*",
      "i8* (i8*)",
      "void (Child*, i8*)",
      "Child* ()",
      "i8*",
      "i8* (i8*, i8*, ...)"))
  }

  "typeDecl" ignore {
    println(cpg.typeDecl.name.p)
    cpg.typeDecl.name("RootClass").l.size shouldBe 1
    cpg.typeDecl.name("Child").l.size shouldBe 1

    cpg.typeDecl.name("RootClass").baseTypeDecl.l.size shouldBe 0
    cpg.typeDecl.name("Child").baseTypeDecl.l.size shouldBe 1
    cpg.typeDecl.name("Child").baseTypeDecl.name("RootClass").l.size shouldBe 1

    cpg.typeDecl.name("RootClass").derivedTypeDecl.l.size shouldBe 1
    cpg.typeDecl.name("RootClass").derivedTypeDecl.name("Child").l.size shouldBe 1
    cpg.typeDecl.name("Child").derivedTypeDecl.l.size shouldBe 0
  }
}
