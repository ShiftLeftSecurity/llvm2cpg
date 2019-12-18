package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._

/*
call void asm sideeffect "add %al, (%rax)", "~{dirflag},~{fpsr},~{flags}"()
ret void
 */

class LLVM_InlineAsmTest extends CPGMatcher {
  private val cpg = CpgLoader.load(TestCpgPaths.LLVM_InlineAsmTestCPG)

  "types" in {
    validateTypes(cpg, Set("ANY", "void ()*", "void", "void ()"))
  }

  "AST" in {
    val block = cpg.method.name("inline_asm").block.head
    val asmCall = block.start.astChildren.isCall.head
    asmCall.methodFullName shouldBe "indirect_call"
    val asm = asmCall.start.astChildren.isLiteral.head
    asm.code shouldBe "add %al, (%rax)"
  }
}
