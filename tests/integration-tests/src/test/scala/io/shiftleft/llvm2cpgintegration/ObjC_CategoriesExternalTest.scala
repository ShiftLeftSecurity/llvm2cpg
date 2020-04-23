package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._
import org.scalatest.BeforeAndAfterAll

class ObjC_CategoriesExternalTest extends CPGMatcher with BeforeAndAfterAll {
  private val cpg = CpgLoader.load(TestCpgPaths.ObjC_CategoriesExternalTestCPG)
  override def beforeAll(): Unit = {
    CpgEnhancer.enhanceCPG(cpg)
  }

  "class typeDecl" in {
    cpg.typeDecl.nameExact("RootClass").l.size shouldBe 1
    cpg.typeDecl.nameExact("RootClass").baseTypeDecl.l.size shouldBe 0
    cpg.typeDecl.nameExact("RootClass").derivedTypeDecl.l.size shouldBe 0

    val rootClass = cpg.typeDecl.nameExact("RootClass").head
    rootClass.start.method.l.size shouldBe 0
    rootClass.start.boundMethod.l.size shouldBe 1
    rootClass.isExternal shouldBe true

    rootClass.start.boundMethod.nameExact("doSomething").l.size shouldBe 1
    val m = rootClass.start.boundMethod.nameExact("doSomething").head
    m.fullName shouldBe "-[RootClass(SomeCategory) doSomething]"
  }

  "metaclass typeDecl" in {
    cpg.typeDecl.nameExact("RootClass$").l.size shouldBe 1
    cpg.typeDecl.nameExact("RootClass$").baseTypeDecl.l.size shouldBe 0
    cpg.typeDecl.nameExact("RootClass$").derivedTypeDecl.l.size shouldBe 0

    val rootClass = cpg.typeDecl.nameExact("RootClass$").head
    rootClass.start.method.l.size shouldBe 0
    rootClass.start.boundMethod.l.size shouldBe 1
    rootClass.isExternal shouldBe true

    rootClass.start.boundMethod.nameExact("doSomethingElse").l.size shouldBe 1
    val m = rootClass.start.boundMethod.nameExact("doSomethingElse").head
    m.fullName shouldBe "+[RootClass(SomeCategory) doSomethingElse]"
  }

  "category typeDecl" in {
    cpg.typeDecl.nameExact("RootClass(SomeCategory)").l.size shouldBe 1
    cpg.typeDecl.nameExact("RootClass(SomeCategory)").baseTypeDecl.l.size shouldBe 0
    cpg.typeDecl.nameExact("RootClass(SomeCategory)").derivedTypeDecl.l.size shouldBe 0

    val category = cpg.typeDecl.nameExact("RootClass(SomeCategory)").head
    category.start.method.l.size shouldBe 2
    category.start.boundMethod.l.size shouldBe 0

    category.start.method.nameExact("doSomething").l.size shouldBe 1
    val m1 = category.start.method.nameExact("doSomething").head
    m1.fullName shouldBe "-[RootClass(SomeCategory) doSomething]"
    category.start.method.nameExact("doSomethingElse").l.size shouldBe 1
    val m2 = category.start.method.nameExact("doSomethingElse").head
    m2.fullName shouldBe "+[RootClass(SomeCategory) doSomethingElse]"
  }
}
