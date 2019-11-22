package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._

class CXX_ClassMethodNamesTest extends CPGMatcher {
  private val cpg = CpgLoader.load(TestCpgPaths.CXX_ClassMethodNamesTestCPG)

  "methods" in {
    val method_1 = cpg.method.fullNameExact("Simple::sayHello(int)").head
    method_1.name shouldBe "sayHello"
  }
}
