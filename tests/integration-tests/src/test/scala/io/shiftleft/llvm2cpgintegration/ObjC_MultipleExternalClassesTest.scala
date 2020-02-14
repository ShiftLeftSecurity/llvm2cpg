package io.shiftleft.llvm2cpgintegration

import io.shiftleft.SerializedCpg
import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._
import io.shiftleft.semanticcpg.layers.EnhancementRunner
import org.scalatest.BeforeAndAfterAll

class ObjC_MultipleExternalClassesTest extends CPGMatcher with BeforeAndAfterAll {
  private val cpg = CpgLoader.load(TestCpgPaths.ObjC_MultipleExternalClassesTestCPG)
  override def beforeAll(): Unit = {
    val enhancement = new EnhancementRunner()
    enhancement.run(cpg, new SerializedCpg())
  }

  "typeDecl" in {
    cpg.typeDecl.nameExact("RootClass").l.size shouldBe 1
    val root = cpg.typeDecl.nameExact("RootClass").head
    root.isExternal shouldBe true
    root.start.method.l shouldBe empty
    root.start.boundMethod.l shouldBe empty

    cpg.typeDecl.nameExact("RootClass").baseTypeDecl.l.size shouldBe 0
    cpg.typeDecl.nameExact("Child").baseTypeDecl.l.size shouldBe 1
    cpg.typeDecl.nameExact("Child").baseTypeDecl.nameExact("RootClass").l.size shouldBe 1
    cpg.typeDecl.nameExact("Child2").baseTypeDecl.l.size shouldBe 1
    cpg.typeDecl.nameExact("Child2").baseTypeDecl.nameExact("RootClass").l.size shouldBe 1

    cpg.typeDecl.nameExact("RootClass").derivedTypeDecl.l.size shouldBe 2
    cpg.typeDecl.nameExact("RootClass").derivedTypeDecl.nameExact("Child").l.size shouldBe 1
    cpg.typeDecl.nameExact("Child").derivedTypeDecl.l.size shouldBe 0
    cpg.typeDecl.nameExact("RootClass").derivedTypeDecl.nameExact("Child2").l.size shouldBe 1
    cpg.typeDecl.nameExact("Child2").derivedTypeDecl.l.size shouldBe 0
  }

}
