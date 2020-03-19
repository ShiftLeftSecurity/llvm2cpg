package io.shiftleft.llvm2cpgintegration

import io.shiftleft.SerializedCpg
import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._
import io.shiftleft.semanticcpg.layers.EnhancementRunner
import org.scalatest.BeforeAndAfterAll

class ObjC_ExternallyDefinedClassTest extends CPGMatcher with BeforeAndAfterAll {
  private val cpg = CpgLoader.load(TestCpgPaths.ObjC_ExternallyDefinedClassTestCPG)
  override def beforeAll(): Unit = {
    val enhancement = new EnhancementRunner()
    enhancement.run(cpg, new SerializedCpg())
  }

  "typeDecl" in {
    cpg.typeDecl.nameExact("RootClass").l.size shouldBe 1
    cpg.typeDecl.nameExact("Child").l.size shouldBe 1
    cpg.typeDecl.nameExact("RootClass$").l.size shouldBe 1
    cpg.typeDecl.nameExact("Child$").l.size shouldBe 1

    val root = cpg.typeDecl.nameExact("RootClass").head
    root.isExternal shouldBe false
    root.start.method.size shouldBe 1
    root.start.boundMethod.size shouldBe 0

    val rootMeta = cpg.typeDecl.nameExact("RootClass$").head
    rootMeta.isExternal shouldBe false
    rootMeta.start.method.size shouldBe 0
    rootMeta.start.boundMethod.size shouldBe 1

    val child = cpg.typeDecl.nameExact("Child").head
    child.isExternal shouldBe false
    child.start.method.size shouldBe 0
    child.start.boundMethod.size shouldBe 0

    val childMeta = cpg.typeDecl.nameExact("Child$").head
    childMeta.isExternal shouldBe false
    childMeta.start.method.size shouldBe 0
    childMeta.start.boundMethod.size shouldBe 1

    cpg.typeDecl.nameExact("RootClass").baseTypeDecl.size shouldBe 0
    cpg.typeDecl.nameExact("Child").baseTypeDecl.size shouldBe 1
    cpg.typeDecl.nameExact("Child").baseTypeDecl.nameExact("RootClass").size shouldBe 1

    cpg.typeDecl.nameExact("RootClass").derivedTypeDecl.size shouldBe 1
    cpg.typeDecl.nameExact("RootClass").derivedTypeDecl.nameExact("Child").size shouldBe 1
    cpg.typeDecl.nameExact("Child").derivedTypeDecl.size shouldBe 0
  }

}
