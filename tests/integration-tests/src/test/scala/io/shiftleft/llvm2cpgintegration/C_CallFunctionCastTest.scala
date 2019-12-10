package io.shiftleft.llvm2cpgintegration

import io.shiftleft.SerializedCpg
import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._
import io.shiftleft.semanticcpg.layers.EnhancementRunner

class C_CallFunctionCastTest extends CPGMatcher {
  private val cpg = CpgLoader.load(TestCpgPaths.C_CallFunctionCastTestCPG)

  "calledMethod" in {
    val enhancement = new EnhancementRunner()
    enhancement.run(cpg, new SerializedCpg())

    cpg.method.nameExact("ecall").l.size shouldBe 1
    cpg.method.nameExact("use").l.size shouldBe 1

    val call = cpg.method.nameExact("ecall").head
    val use = cpg.method.nameExact("use").head

    val resolver : ICallResolver = NoResolve
    use.start.callOut.nameExact("ecall").calledMethod(resolver).head shouldBe call
  }
}
