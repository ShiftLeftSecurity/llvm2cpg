package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._

class C_ReturnGlobalTest extends CPGMatcher {
  private val cpg = CpgLoader.load(TestCpgPaths.C_ReturnGlobalCPG)
  private val methodName = "basic_c_support"

  "types" in {
    validateTypes(cpg, Set("ANY", "i32", "i32 ()"))
  }

  "method AST" in {
    /*
      %0 = load i32, i32* @x
      ret i32 %0
    */

    val method = cpg.method.name(methodName).head
    val block = method.start.block.head
    block.start.astChildren.l.size shouldBe 3
    block.start.astChildren.isCall.l.size shouldBe 1
    block.start.astChildren.isReturnNode.l.size shouldBe 1

    val tmp = block.start.local.head

    {
      // %0 = load i32, i32* %x.addr
      val assignCall = block.start.astChildren.isCall.l.head
      assignCall.name shouldBe "<operator>.assignment"
      assignCall.order shouldBe 1
      assignCall.argumentIndex shouldBe 1
      assignCall.start.astChildren.l.size shouldBe 2

      assignCall.start.astChildren.isIdentifier.l.size shouldBe 1
      val lhs =  assignCall.start.astChildren.isIdentifier.head
      lhs.name shouldBe "tmp"
      lhs.typeFullName shouldBe "i32"
      lhs.start.refsTo.l.size shouldBe 1
      lhs.start.refsTo.head shouldBe tmp
      lhs.order shouldBe 1
      lhs.argumentIndex shouldBe 1

      assignCall.start.astChildren.isCall.l.size shouldBe 1
      val rhs =  assignCall.start.astChildren.isCall.head
      rhs.name shouldBe "<operator>.indirection"
      rhs.typeFullName shouldBe "i32"
      rhs.order shouldBe 2
      rhs.argumentIndex shouldBe 2
      rhs.start.astChildren.l.size shouldBe 1
      rhs.start.astChildren.isLiteral.l.size shouldBe 1
      val argument = rhs.start.astChildren.isLiteral.head
      argument.code shouldBe "15"
      argument.typeFullName shouldBe "i32"
      argument.order shouldBe 1
      argument.argumentIndex shouldBe 1
    }

    {
      // ret i32 %0
      val ret = block.start.astChildren.isReturnNode.l.last
      ret.code shouldBe "return"
      ret.order shouldBe 2
      ret.argumentIndex shouldBe 2

      ret.start.astChildren.l.size shouldBe 1
      ret.start.astChildren.isIdentifier.l.size shouldBe 1
      val retVal = ret.start.astChildren.isIdentifier.head

      retVal.name shouldBe "tmp"
      retVal.order shouldBe 1
      retVal.argumentIndex shouldBe 1
      retVal.start.refsTo.l.size shouldBe 1
      retVal.start.refsTo.head shouldBe tmp
    }

  }

  "CFG" in {
    /*
      %0 = load i32, i32* @x
      ret i32 %0
    */

    val method = cpg.method.name(methodName).head
    val block = method.start.block.head

    val assignLoadCall = block.start.astChildren.isCall.l.head

    {
      // %0 = load i32, i32* @x
      val lhs =  assignLoadCall.start.astChildren.isIdentifier.head
      val rhs =  assignLoadCall.start.astChildren.isCall.head
      val ref = rhs.start.astChildren.isLiteral.head

      lhs.start.cfgNext.head shouldBe ref
      ref.start.cfgNext.head shouldBe rhs
      rhs.start.cfgNext.head shouldBe assignLoadCall
    }

    {
      // ret i32 %0
      val ret = block.start.astChildren.isReturnNode.l.last
      val retVal = ret.start.astChildren.isIdentifier.head
      retVal.start.cfgNext.head shouldBe ret
      assignLoadCall.start.cfgNext.head shouldBe retVal

      val methodReturn = method.start.methodReturn.head
      methodReturn.start.cfgLast.head shouldBe ret
    }
  }

}
