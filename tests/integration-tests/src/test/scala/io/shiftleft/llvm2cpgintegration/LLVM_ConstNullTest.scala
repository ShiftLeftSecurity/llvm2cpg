package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._

class LLVM_ConstNullTest extends CPGMatcher {
  private val cpg = CpgLoader.load(TestCpgPaths.LLVM_ConstNullCPG)

  "types" in {
    validateTypes(cpg, Set("ANY", "i32*"))
  }

  "AST" in {
    // ret i32* null
    val method = cpg.method.name("foo").head
    val block = method.start.block.head
    val ret = block.start.astChildren.isReturnNode.head
    val retVal = ret.start.astChildren.isLiteral.head
    retVal.code shouldBe "nullptr"
    retVal.typeFullName shouldBe "i32*"
  }
}
