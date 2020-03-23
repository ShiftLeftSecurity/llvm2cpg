package io.shiftleft.llvm2cpgintegration

import io.shiftleft.SerializedCpg
import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._
import io.shiftleft.semanticcpg.layers.EnhancementRunner
import org.scalatest.BeforeAndAfterAll

class ObjC_CategoriesTest extends CPGMatcher with BeforeAndAfterAll {
  private val cpg = CpgLoader.load(TestCpgPaths.ObjC_CategoriesTestCPG)
  override def beforeAll(): Unit = {
    val enhancement = new EnhancementRunner()
    enhancement.run(cpg, new SerializedCpg())
  }

  "class typeDecl" in {
    cpg.typeDecl.nameExact("RootClass").l.size shouldBe 1
    cpg.typeDecl.nameExact("RootClass").baseTypeDecl.l.size shouldBe 0
    cpg.typeDecl.nameExact("RootClass").derivedTypeDecl.l.size shouldBe 0

    val rootClass = cpg.typeDecl.nameExact("RootClass").head
    rootClass.start.method.l.size shouldBe 2
    rootClass.start.boundMethod.l.size shouldBe 2

    rootClass.start.boundMethod.nameExact("init").l.size shouldBe 1
    val m1 = rootClass.start.boundMethod.nameExact("init").head
    m1.fullName shouldBe "-[RootClass init]"
    rootClass.start.boundMethod.nameExact("doSomething").l.size shouldBe 1
    val m2 = rootClass.start.boundMethod.nameExact("doSomething").head
    m2.fullName shouldBe "-[RootClass(SomeCategory) doSomething]"
  }

  "metaclass typeDecl" in {
    cpg.typeDecl.nameExact("RootClass$").l.size shouldBe 1
    cpg.typeDecl.nameExact("RootClass$").baseTypeDecl.l.size shouldBe 0
    cpg.typeDecl.nameExact("RootClass$").derivedTypeDecl.l.size shouldBe 0

    val rootClass = cpg.typeDecl.nameExact("RootClass$").head
    rootClass.start.method.l.size shouldBe 0
    rootClass.start.boundMethod.l.size shouldBe 2

    rootClass.start.boundMethod.nameExact("alloc").l.size shouldBe 1
    val m1 = rootClass.start.boundMethod.nameExact("alloc").head
    m1.fullName shouldBe "+[RootClass alloc]"
    rootClass.start.boundMethod.nameExact("doSomethingElse").l.size shouldBe 1
    val m2 = rootClass.start.boundMethod.nameExact("doSomethingElse").head
    m2.fullName shouldBe "+[RootClass(SomeCategory) doSomethingElse]"
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
