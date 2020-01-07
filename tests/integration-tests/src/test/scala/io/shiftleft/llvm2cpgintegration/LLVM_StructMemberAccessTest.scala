package io.shiftleft.llvm2cpgintegration

import io.shiftleft.SerializedCpg
import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._
import io.shiftleft.semanticcpg.layers.EnhancementRunner

class LLVM_StructMemberAccessTest extends CPGMatcher {
  private val cpg = CpgLoader.load(TestCpgPaths.LLVM_StructMemberAccessTestCPG)

  "member access" in {
    val enhancement = new EnhancementRunner()
    enhancement.run(cpg, new SerializedCpg())

    val gepAssignment = cpg.method.nameExact("test").block.astChildren.isCall.head
    val topGepCall = gepAssignment.start.astChildren.isCall.head
    topGepCall.typeFullName shouldBe "i16*"
    topGepCall.name shouldBe "<operator>.memberAccess"

    val structGep = topGepCall.start.astChildren.isCall.head
    structGep.typeFullName shouldBe "struct.kcdata_subtype_descriptor"
    structGep.name shouldBe "<operator>.computedMemberAccess"

    val arrayGep = structGep.start.astChildren.isCall.head
    arrayGep.typeFullName shouldBe "[7 x struct.kcdata_subtype_descriptor]"
    arrayGep.name shouldBe "<operator>.computedMemberAccess"
  }
}
