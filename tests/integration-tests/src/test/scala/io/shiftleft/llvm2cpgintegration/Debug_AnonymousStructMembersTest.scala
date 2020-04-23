package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._
import org.scalatest.BeforeAndAfterAll

class Debug_AnonymousStructMembersTest extends CPGMatcher with BeforeAndAfterAll {
  private val cpg = CpgLoader.load(TestCpgPaths.Debug_AnonymousStructMembersTestCPG)
  override def beforeAll(): Unit = {
    CpgEnhancer.enhanceCPG(cpg)
  }

  "types" in {
    val types = List("Point", "PointPointer")
    types.foreach(typeName => {
      cpg.typeDecl.nameExact(typeName).member.l should have length(3)
      cpg.typeDecl.nameExact(typeName).member.nameExact("x").l should have length(1)
      cpg.typeDecl.nameExact(typeName).member.nameExact("y").l should have length(1)
      cpg.typeDecl.nameExact(typeName).member.nameExact("something").l should have length(1)
    })

    val anonymousStruct = cpg.typeDecl.name("anon").head
    anonymousStruct.start.member.l should have length(2)
  }
}
