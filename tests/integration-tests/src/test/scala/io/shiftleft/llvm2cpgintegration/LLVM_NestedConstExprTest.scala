package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.codepropertygraph.generated.nodes.{Expression, Unknown}
import io.shiftleft.semanticcpg.language._

// ret i32 zext (i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null) to i32)

class LLVM_NestedConstExprTest extends CPGMatcher {
  private val cpg = CpgLoader.load(TestCpgPaths.LLVM_NestedConstExprTestCPG)
  private val methodName = "__gthread_active_p()"

  "types" in {
    validateTypes(cpg, Set("ANY", "i1", "i8*", "i32", "i32*", "void (i8*)*", "i32 ()", "i32 (i32*, void (i8*)*)"))
  }

  "AST" in {
    val method = cpg.method.name(methodName).head
    val block = method.start.block.head
    block.start.astChildren.l.size shouldBe 1

    val ret = block.start.astChildren.isReturn.head

    ret.start.astChildren.l.size shouldBe 1
    val zext = ret.start.astChildren.isCall.head
    zext.name shouldBe "<operator>.cast"
    zext.methodFullName shouldBe "<operator>.cast"
    zext.start.astChildren.l.size shouldBe 1

    val icmp = zext.start.astChildren.isCall.head
    icmp.name shouldBe "<operator>.notEquals"
    icmp.methodFullName shouldBe "<operator>.notEquals"
    icmp.start.astChildren.l.size shouldBe 2

    val icmpBitcast = icmp.start.astChildren.isCall.head
    icmpBitcast.name shouldBe "<operator>.cast"
    icmpBitcast.methodFullName shouldBe "<operator>.cast"
    icmpBitcast.start.astChildren.l.size shouldBe 1

    val bitcastArg = icmpBitcast.start.astChildren.isMethodRef.head
    bitcastArg.methodFullName shouldBe "__pthread_key_create"

    val icmpNull = icmp.start.astChildren.isLiteral.head
    icmpNull.code shouldBe "nullptr"
    icmpNull.start.astChildren.l.size shouldBe 0
  }

  "CFG" in {
    val method = cpg.method.name(methodName).head
    val block = method.start.block.head
    val ret = block.start.astChildren.isReturn.head
    val zext = ret.start.astChildren.isCall.head
    val icmp = zext.start.astChildren.isCall.head
    val icmpBitcast = icmp.start.astChildren.isCall.head
    val bitcastArg = icmpBitcast.start.astChildren.head.asInstanceOf[Expression]
    val icmpNull = icmp.start.astChildren.isLiteral.head

    method.start.cfgFirst.head shouldBe bitcastArg
    bitcastArg.start.cfgNext.head shouldBe icmpBitcast
    icmpBitcast.start.cfgNext.head shouldBe icmpNull
    icmpNull.start.cfgNext.head shouldBe icmp
    icmp.start.cfgNext.head shouldBe zext
    zext.start.cfgNext.head shouldBe ret
  }

}
