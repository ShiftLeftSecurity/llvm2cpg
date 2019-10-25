package io.shiftleft.llvm2cpgintegration

import io.shiftleft.SerializedCpg
import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._
import io.shiftleft.semanticcpg.layers.EnhancementRunner

class DataFlow_CallerArgumentsTest extends CPGMatcher {
  private val cpg = CpgLoader.load(TestCpgPaths.DataFlow_CallerArgumentsTestCPG)

  "data flow" in {
    val enhancement = new EnhancementRunner()
    enhancement.run(cpg, new SerializedCpg())

    val callee = cpg.method.name("foo").head

    callee.start.parameter.l.size shouldBe 1
    val arg = callee.start.parameter.argument.isLiteral.head
    arg.code shouldBe "42"
  }
}
