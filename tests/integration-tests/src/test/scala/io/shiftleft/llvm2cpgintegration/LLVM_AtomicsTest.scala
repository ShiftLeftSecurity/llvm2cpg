package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._
import org.scalatest.{Matchers, WordSpec}

class LLVM_AtomicsTest extends CPGMatcher {
  private val cpg = CpgLoader.load(TestCpgPaths.LLVM_AtomicsTestCPG)

  "types" in {
    validateTypes(cpg, Set("ANY", "{ i32, i1 }", "i32", "i32*", "void", "void (i32*)", "i32 (i32*)"))
  }

  "AST" in {
    val inc = cpg.method.name("atomic_inc").head
    inc.start.block.astChildren.astChildren.isCall.name("<operator>.atomicAddition").l.size  shouldBe 1

    cpg.method.name("atomiccmpxchg").head.start.ast.isCall.name("<operator>.cmpxchg").l.size shouldBe 1
  }

  "CFG" in {
    val inc = cpg.method.name("atomic_inc").head
    val atomicinc = inc.start.ast.isCall.name("<operator>.atomicAddition").head
    atomicinc.start.cfgNext.head shouldBe inc.start.ast.isCall.name("<operator>.assignment").head
    atomicinc.start.cfgPrev.head shouldBe inc.start.ast.isLiteral.code("1").head
  }
}