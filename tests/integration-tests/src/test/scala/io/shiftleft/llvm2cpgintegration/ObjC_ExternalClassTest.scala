package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._
import org.scalatest.BeforeAndAfterAll

class ObjC_ExternalClassTest extends CPGMatcher with BeforeAndAfterAll {
  private val cpg = CpgLoader.load(TestCpgPaths.ObjC_ExternalClassTestCPG)
  override def beforeAll(): Unit = {
    CpgEnhancer.enhanceCPG(cpg)
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

    cpg.typeDecl.nameExact("RootClass").derivedTypeDecl.l.size shouldBe 1
    cpg.typeDecl.nameExact("RootClass").derivedTypeDecl.nameExact("Child").l.size shouldBe 1
    cpg.typeDecl.nameExact("Child").derivedTypeDecl.l.size shouldBe 0
  }

}
