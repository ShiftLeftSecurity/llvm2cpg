package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._

class DataFlow_CallerTest extends CPGMatcher {
  private val cpg = CpgLoader.load(TestCpgPaths.DataFlow_CallerTestCPG)

  "data flow" in {
    CpgEnhancer.enhanceCPG(cpg)

    val resolver : ICallResolver = NoResolve
    val callee = cpg.method.name("foo").head
    val caller = cpg.method.name("bar").head

    callee.start.caller(resolver).head shouldBe caller
    caller.start.callee(resolver).head shouldBe callee

    val callNode = caller.start.block.astChildren.isCall.head
    callNode.start.calledMethod(resolver).head shouldBe callee
  }
}
