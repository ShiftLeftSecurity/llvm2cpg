package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._
import org.scalatest.{Matchers, WordSpec}

class LLVM_AtomicsTest extends WordSpec with Matchers {
  private val cpg = CpgLoader.load(TestCpgPaths.LLVM_AtomicsCPG)
  
  "AST" in {
      
    val inc = cpg.method.name("atomic_inc").head
    inc.start.block.astChildren.astChildren.call.name("atomicrmwadd").l.size  shouldBe 1

    cpg.method.name("atomiccmpxchg").head.start.ast.call.name("cmpxchg").l.size shouldBe 1
  }

  "CFG" in {
    val inc = cpg.method.name("atomic_inc").head
    val atomicinc = inc.start.ast.call.name("atomicrmwadd").head
    atomicinc.start.cfgNext.head shouldBe inc.start.ast.call.name("=").head
    atomicinc.start.cfgPrev.head shouldBe inc.start.ast.literal.code("1").head
  }
}