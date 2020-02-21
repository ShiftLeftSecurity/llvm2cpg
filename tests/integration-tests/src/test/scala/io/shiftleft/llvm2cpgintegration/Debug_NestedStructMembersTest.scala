package io.shiftleft.llvm2cpgintegration

import io.shiftleft.SerializedCpg
import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._
import io.shiftleft.semanticcpg.layers.EnhancementRunner
import org.scalatest.BeforeAndAfterAll

class Debug_NestedStructMembersTest extends CPGMatcher with BeforeAndAfterAll {
  private val cpg = CpgLoader.load(TestCpgPaths.Debug_NestedStructMembersTestCPG)
  override def beforeAll(): Unit = {
    val enhancement = new EnhancementRunner()
    enhancement.run(cpg, new SerializedCpg())
  }

  "types" in {
    val person = cpg.typeDecl.name("Person").head
    person.start.member.l should have length(3)
    person.start.member.name("age").l should have length(1)
    person.start.member.name("name").l should have length(1)
    person.start.member.name("hobbies").l should have length(1)

    val hobby = cpg.typeDecl.name("Hobby").head
    hobby.start.member.l should have length(2)
    hobby.start.member.name("length").l should have length(1)
    hobby.start.member.name("hobbyName").l should have length(1)

    val name = cpg.typeDecl.name("Name").head
    name.start.member.l should have length(2)
    name.start.member.name("length").l should have length(1)
    name.start.member.name("buffer").l should have length(1)
  }
}
