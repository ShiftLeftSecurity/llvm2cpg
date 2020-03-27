package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._

class LLVM_StoreConstExprTest extends CPGMatcher {
  private val cpg = CpgLoader.load(TestCpgPaths.LLVM_StoreConstExprTestCPG)
  private val methodName = "store_const"

  "types" in {
    validateTypes(cpg, List("ANY", "i8", "i8*", "[11 x i8]", "i64", "void", "void ()"))
  }

  "AST" in {
    /*
      store i8 42, i8* getelementptr ([11 x i8], [11 x i8]* @.str, i64 0, i64 1)
      ret void
    */
    val method = cpg.method.name(methodName).head
    val block = method.start.block.head
    block.start.astChildren.l.size shouldBe 2

    val assignStore = block.start.astChildren.isCall.head
    val store = assignStore.start.astChildren.isCall.head
    val rhs = store.start.astChildren.isCall.head
    rhs.name shouldBe "<operator>.indexAccess"
    rhs.methodFullName shouldBe "<operator>.indexAccess"
  }
}
