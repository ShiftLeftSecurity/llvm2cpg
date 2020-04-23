package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._
import org.scalatest.BeforeAndAfterAll

class ObjC_ClassesTest extends CPGMatcher with BeforeAndAfterAll {
  private val cpg = CpgLoader.load(TestCpgPaths.ObjC_ClassesTestCPG)
  override def beforeAll(): Unit = {
    CpgEnhancer.enhanceCPG(cpg)
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

  "call site" in {
    val method =  cpg.method.nameExact("useChild").head
    method.start.callOut.name("newChild").argument.size shouldBe 2

    val doSomething = cpg.method.fullNameExact("-[Child doSomething]").head
    val newChild = cpg.method.fullNameExact("+[Child newChild]").head
    val resolver : ICallResolver = NoResolve

    method.start.callOut.name("doSomething").calledMethod(resolver).head shouldBe doSomething
    method.start.callOut.name("doSomething").argument.size shouldBe 2

//  This works with Ocular, but not yet supported by codepropertygraph
//
//    method.start.callOut.name("newChild").calledMethod(resolver).head shouldBe newChild
//    method.start.callOut.name("newChild").argument.size shouldBe 2
  }
}
