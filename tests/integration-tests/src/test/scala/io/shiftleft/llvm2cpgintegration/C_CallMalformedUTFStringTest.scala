package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._

class C_CallMalformedUTFStringTest extends CPGMatcher {
  private val cpg = CpgLoader.load(TestCpgPaths.C_CallMalformedUTFStringTestCPG)

  "literals" in {
    cpg.literal.l.size shouldBe 1
    val literal = cpg.literal.head
    literal.code shouldBe "0x060x090x860x01"
  }

}
