package io.shiftleft.llvm2cpgintegration

import io.shiftleft.SerializedCpg
import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._
import io.shiftleft.semanticcpg.layers.EnhancementRunner
import org.scalatest.BeforeAndAfterAll

class ObjC_ClassesTest extends CPGMatcher with BeforeAndAfterAll {
  private val cpg = CpgLoader.load(TestCpgPaths.ObjC_ClassesTestCPG)
  override def beforeAll(): Unit = {
    val enhancement = new EnhancementRunner()
    enhancement.run(cpg, new SerializedCpg())
  }

  "class typeDecl" in {
    cpg.typeDecl.nameExact("RootClass").l.size shouldBe 1
    cpg.typeDecl.nameExact("Child").l.size shouldBe 1

    cpg.typeDecl.nameExact("RootClass").baseTypeDecl.l.size shouldBe 0
    cpg.typeDecl.nameExact("Child").baseTypeDecl.l.size shouldBe 1
    cpg.typeDecl.nameExact("Child").baseTypeDecl.nameExact("RootClass").l.size shouldBe 1

    cpg.typeDecl.nameExact("RootClass").derivedTypeDecl.l.size shouldBe 1
    cpg.typeDecl.nameExact("RootClass").derivedTypeDecl.nameExact("Child").l.size shouldBe 1
    cpg.typeDecl.nameExact("Child").derivedTypeDecl.l.size shouldBe 0
  }

  "metaclass typeDecl" in {
    cpg.typeDecl.nameExact("RootClass$").l.size shouldBe 1
    cpg.typeDecl.nameExact("Child$").l.size shouldBe 1

    cpg.typeDecl.nameExact("RootClass$").baseTypeDecl.l.size shouldBe 0
    cpg.typeDecl.nameExact("Child$").baseTypeDecl.l.size shouldBe 1
    cpg.typeDecl.nameExact("Child$").baseTypeDecl.nameExact("RootClass$").l.size shouldBe 1

    cpg.typeDecl.nameExact("RootClass$").derivedTypeDecl.l.size shouldBe 1
    cpg.typeDecl.nameExact("RootClass$").derivedTypeDecl.nameExact("Child$").l.size shouldBe 1
    cpg.typeDecl.nameExact("Child$").derivedTypeDecl.l.size shouldBe 0
  }

}
