package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.codepropertygraph.generated.nodes.{Expression, Unknown}
import io.shiftleft.semanticcpg.language._
import overflowdb.traversal.NodeOps

class LLVM_UnreachableTest extends CPGMatcher {
  private val cpg = CpgLoader.load(TestCpgPaths.LLVM_UnreachableTestCPG)
  private val methodName = "unreachable"

  "types" in {
    validateTypes(cpg, List("ANY", "void", "void ()"))
  }

  "AST" in {
    val method = cpg.method.name(methodName).head
    val block = method.start.block.head
    block.start.astChildren.l.size shouldBe 3

    val noop = block.start.astChildren.head.asInstanceOf[Unknown]
    noop.code shouldBe "noop"

    val unreachable = block.start.astChildren.l.apply(1).asInstanceOf[Unknown]
    unreachable.code shouldBe "unreachable"

    val ret = block.start.astChildren.isReturn.head
    ret.code shouldBe "return"
  }

  "CFG" in {
    val method = cpg.method.name(methodName).head
    val block = method.start.block.head

    val noop = block.start.astChildren.head.asInstanceOf[Expression]
    val unreachable = block.start.astChildren.l.apply(1).asInstanceOf[Expression]
    val ret = block.start.astChildren.isReturn.head

    method.start.cfgFirst.head shouldBe noop
    noop.start.cfgNext.head shouldBe unreachable
    unreachable.start.cfgNext.l.size shouldBe 0
    ret.start.cfgPrev.l.size shouldBe 0
  }

}
