package io.shiftleft.llvm2cpgintegration

import io.shiftleft.SerializedCpg
import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._
import io.shiftleft.semanticcpg.layers.EnhancementRunner

class ObjC_ClassesTest extends CPGMatcher {
  private val cpg = CpgLoader.load(TestCpgPaths.ObjC_ClassesTestCPG)

  "typeDecl" in {
    val enhancement = new EnhancementRunner()
    enhancement.run(cpg, new SerializedCpg())

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
