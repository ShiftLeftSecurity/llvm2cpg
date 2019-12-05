package io.shiftleft.llvm2cpgintegration

import io.shiftleft.SerializedCpg
import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._
import io.shiftleft.semanticcpg.layers.EnhancementRunner
import org.scalatest.BeforeAndAfterAll

class ObjC_MethodsTest extends CPGMatcher with BeforeAndAfterAll {
  private val cpg = CpgLoader.load(TestCpgPaths.ObjC_MethodsTestCPG)
  override def beforeAll(): Unit = {
    val enhancement = new EnhancementRunner()
    enhancement.run(cpg, new SerializedCpg())
  }

  "metaclass methods" in {
    val rootClass = cpg.typeDecl.nameExact("RootClass$").head
    rootClass.start.method.l.size shouldBe 0
    rootClass.start.boundMethod.l.size shouldBe 1

    rootClass.start.boundMethod.nameExact("overridden").l.size shouldBe 1
    val overriden = rootClass.start.boundMethod.nameExact("overridden").head
    overriden.fullName shouldBe "+[RootClass overridden]"

    val child = cpg.typeDecl.nameExact("Child$").head
    child.start.method.l.size shouldBe 0
    child.start.boundMethod.l.size shouldBe 2

    child.start.boundMethod.nameExact("newChild").l.size shouldBe 1
    val boundNewChild = child.start.boundMethod.nameExact("newChild").head
    boundNewChild.fullName shouldBe "+[Child newChild]"

    child.start.boundMethod.nameExact("overridden").l.size shouldBe 1
    val overridenChild = child.start.boundMethod.nameExact("overridden").head
    overridenChild.fullName shouldBe "+[Child overridden]"
  }

  "class methods" in {
    val rootClass = cpg.typeDecl.nameExact("RootClass").head

    rootClass.start.method.l.size shouldBe 3
    rootClass.start.method.nameExact("inherited").l.size shouldBe 1
    val inheritedRoot = rootClass.start.method.nameExact("inherited").head
    inheritedRoot.fullName shouldBe "-[RootClass inherited]"

    rootClass.start.method.nameExact("overridden").l.size shouldBe 2
    rootClass.start.method.fullNameExact("+[RootClass overridden]").l.size shouldBe 1
    rootClass.start.method.fullNameExact("-[RootClass overridden]").l.size shouldBe 1

    rootClass.start.boundMethod.l.size shouldBe 2
    rootClass.start.boundMethod.nameExact("inherited").l.size shouldBe 1
    val boundInheritedRoot = rootClass.start.method.nameExact("inherited").head
    boundInheritedRoot.fullName shouldBe "-[RootClass inherited]"

    rootClass.start.boundMethod.nameExact("overridden").l.size shouldBe 1
    val boundOverriddenRoot = rootClass.start.boundMethod.nameExact("overridden").head
    boundOverriddenRoot.fullName shouldBe "-[RootClass overridden]"

    val child = cpg.typeDecl.nameExact("Child").head

    child.start.method.l.size shouldBe 4

    child.start.method.nameExact("doSomething").l.size shouldBe 1
    val doSomething = child.start.method.nameExact("doSomething").head
    doSomething.fullName shouldBe "-[Child doSomething]"

    child.start.method.nameExact("newChild").l.size shouldBe 1
    val newChild = child.start.method.nameExact("newChild").head
    newChild.fullName shouldBe "+[Child newChild]"

    child.start.method.nameExact("overridden").l.size shouldBe 2
    child.start.method.fullNameExact("+[Child overridden]").l.size shouldBe 1
    child.start.method.fullNameExact("-[Child overridden]").l.size shouldBe 1

    child.start.boundMethod.l.size shouldBe 3

    child.start.boundMethod.nameExact("inherited").l.size shouldBe 1
    val boundInheritedChild = child.start.boundMethod.nameExact("inherited").head
    boundInheritedChild.fullName shouldBe "-[RootClass inherited]"

    child.start.boundMethod.nameExact("doSomething").l.size shouldBe 1
    val boundDoSomething = child.start.boundMethod.nameExact("doSomething").head
    boundDoSomething.fullName shouldBe "-[Child doSomething]"

    child.start.boundMethod.nameExact("overridden").l.size shouldBe 1
    val boundOverridden = child.start.boundMethod.nameExact("overridden").head
    boundOverridden.fullName shouldBe "-[Child overridden]"
  }
}
