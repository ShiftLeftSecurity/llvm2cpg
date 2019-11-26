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

  "typeDecl" in {
    cpg.typeDecl.name("RootClass").l.size shouldBe 1
    cpg.typeDecl.name("Child").l.size shouldBe 1

    cpg.typeDecl.name("RootClass").baseTypeDecl.l.size shouldBe 0
    cpg.typeDecl.name("Child").baseTypeDecl.l.size shouldBe 1
    cpg.typeDecl.name("Child").baseTypeDecl.name("RootClass").l.size shouldBe 1

    cpg.typeDecl.name("RootClass").derivedTypeDecl.l.size shouldBe 1
    cpg.typeDecl.name("RootClass").derivedTypeDecl.name("Child").l.size shouldBe 1
    cpg.typeDecl.name("Child").derivedTypeDecl.l.size shouldBe 0
  }

  "methods" in {
    val rootClass = cpg.typeDecl.name("RootClass").head
    rootClass.start.method.name("init").l.size shouldBe 1
    val init = rootClass.start.method.name("init").head
    init.fullName shouldBe "-[RootClass init]"

    rootClass.start.boundMethod.name("init").l.size shouldBe 1
    val boundInit = rootClass.start.boundMethod.name("init").head
    boundInit.fullName shouldBe "-[RootClass init]"

    val child = cpg.typeDecl.name("Child").head
    child.start.method.name("doSomething").l.size shouldBe 1
    val doSomething = child.start.method.name("doSomething").head
    doSomething.fullName shouldBe "-[Child doSomething]"

    child.start.boundMethod.name("doSomething").l.size shouldBe 1
    val boundDoSomething = child.start.boundMethod.name("doSomething").head
    boundDoSomething.fullName shouldBe "-[Child doSomething]"

    child.start.method.name("newChild").l.size shouldBe 1
    val newChild = child.start.method.name("newChild").head
    newChild.fullName shouldBe "+[Child newChild]"

    child.start.boundMethod.name("newChild").l.size shouldBe 1
    val boundNewChild = child.start.boundMethod.name("newChild").head
    boundNewChild.fullName shouldBe "+[Child newChild]"
  }
}
