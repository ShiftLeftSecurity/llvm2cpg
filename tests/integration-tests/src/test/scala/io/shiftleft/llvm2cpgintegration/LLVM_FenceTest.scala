package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._

class LLVM_FenceTest extends CPGMatcher {
  private val cpg = CpgLoader.load(TestCpgPaths.LLVM_FenceTestCpg)

  "types" in {
    validateTypes(cpg, Set("ANY", "void"))
  }

  "CFG" in {
    // ret i32* null
    val foo = cpg.method.name("foo").head
    foo.start.cfgFirst.l.size shouldBe 1
    val f1 = foo.start.cfgFirst.call.name("fence").head
    f1.start.cfgNext.l.size shouldBe 1
    val f2 = f1.start.cfgNext.call.name("fence").head
    f2.start.cfgNext.l.size shouldBe 1
    val f3 = f2.start.cfgNext.call.name("fence").head
    f3.start.cfgNext.l.size shouldBe 1
    f3.start.cfgNext.isReturnNode.l.size shouldBe 1
  }
}
