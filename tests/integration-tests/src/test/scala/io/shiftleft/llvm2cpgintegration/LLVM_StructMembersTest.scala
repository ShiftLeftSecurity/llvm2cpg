package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._

class LLVM_StructMembersTest extends CPGMatcher {
  private val cpg = CpgLoader.load(TestCpgPaths.LLVM_StructMembersTestCPG)

  "struct members" in {
    val color = cpg.typeDecl.name("Color").head
    color.start.member.l should have length(3)
    color.start.member.name("0").head.typeFullName shouldBe "i32"
    color.start.member.name("1").head.typeFullName shouldBe "i32"
    color.start.member.name("2").head.typeFullName shouldBe "i32"

    val colorA = cpg.typeDecl.name("ColorA").head
    colorA.start.member.l should have length(2)
    colorA.start.member.name("0").head.typeFullName shouldBe "Color"
    colorA.start.member.name("1").head.typeFullName shouldBe "float"

    val poly = cpg.typeDecl.name("Poly").head
    poly.start.member.l should have length(2)
    poly.start.member.name("0").head.typeFullName shouldBe "i32"
    poly.start.member.name("1").head.typeFullName shouldBe "anon"

    val anon = cpg.typeDecl.name("anon").head
    anon.start.member.l should have length(1)
    anon.start.member.name("0").head.typeFullName shouldBe "double"
  }
}
