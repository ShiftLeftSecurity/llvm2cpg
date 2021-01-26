package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._
import overflowdb.traversal.NodeOps

class LLVM_StructMemberAccessTest extends CPGMatcher {
  private val cpg = CpgLoader.load(TestCpgPaths.LLVM_StructMemberAccessTestCPG)
  "member access2" in {
    var gep = cpg.method.nameExact("test2").block.astChildren.isCall.head
    gep = gep.start.astChildren.isCall.head
    gep.typeFullName shouldBe "i8*"
    gep.name shouldBe "<operator>.indexAccess"

    gep = gep.start.astChildren.isCall.head
    gep.typeFullName shouldBe "{ i8, i8 }"
    gep.name shouldBe "<operator>.indexAccess"

    gep = gep.start.astChildren.isCall.head
    gep.typeFullName shouldBe "{ i8, { i8, i8 } }"
    gep.name shouldBe "<operator>.getElementPtr"

  }

  "member access" in {
    val gepAssignment = cpg.method.nameExact("test").block.astChildren.isCall.head
    val topGepCall = gepAssignment.start.astChildren.isCall.head
    topGepCall.typeFullName shouldBe "i16*"
    topGepCall.name shouldBe "<operator>.getElementPtr"

    val structGep = topGepCall.start.astChildren.isCall.head
    structGep.typeFullName shouldBe "kcdata_subtype_descriptor"
    structGep.name shouldBe "<operator>.indexAccess"


    val arrayGep = structGep.start.astChildren.isCall.head
    arrayGep.name shouldBe "<operator>.pointerShift"
    arrayGep.typeFullName shouldBe "[7 x kcdata_subtype_descriptor]"
  }
}
