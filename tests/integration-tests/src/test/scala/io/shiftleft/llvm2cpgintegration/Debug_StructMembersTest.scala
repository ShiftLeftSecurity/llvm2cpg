package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._
import org.scalatest.BeforeAndAfterAll

class Debug_StructMembersTest extends CPGMatcher with BeforeAndAfterAll {
  private val cpg = CpgLoader.load(TestCpgPaths.Debug_StructMembersTestCPG)
  override def beforeAll(): Unit = {
    CpgEnhancer.enhanceCPG(cpg)
  }

  "types" in {
    cpg.typeDecl.nameExact("Point").member.l should have length(2)
    cpg.typeDecl.nameExact("Point").member.nameExact("x").l should have length(1)
    cpg.typeDecl.nameExact("Point").member.nameExact("y").l should have length(1)

    cpg.typeDecl.nameExact("PointPointer").member.l should have length(2)
    cpg.typeDecl.nameExact("PointPointer").member.nameExact("x").l should have length(1)
    cpg.typeDecl.nameExact("PointPointer").member.nameExact("y").l should have length(1)

    cpg.typeDecl.nameExact("PointTypedef").member.l should have length(2)
    cpg.typeDecl.nameExact("PointTypedef").member.nameExact("x").l should have length(1)
    cpg.typeDecl.nameExact("PointTypedef").member.nameExact("y").l should have length(1)
  }

  "member access" in {
    val gep1 = cpg.method.name("usePoint").block.astChildren.isCall.order(3).astChildren.isCall.head
    gep1.name shouldBe "<operator>.getElementPtr"
    val gep1Reference = gep1.start.astChildren.isFieldIdentifier.head
    gep1Reference.code shouldBe "x"
    gep1Reference.canonicalName shouldBe "x"

    val gep2 = cpg.method.name("usePoint").block.astChildren.isCall.order(5).astChildren.isCall.head
    gep2.name shouldBe "<operator>.getElementPtr"
    val gep2Reference = gep2.start.astChildren.isFieldIdentifier.head
    gep2Reference.code shouldBe "y"
    gep2Reference.canonicalName shouldBe "y"
  }
}
