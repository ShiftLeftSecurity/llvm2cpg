package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.codepropertygraph.generated.nodes.{Expression, Unknown}
import io.shiftleft.semanticcpg.language._
import overflowdb.traversal.NodeOps

class LLVM_SwitchesTest extends CPGMatcher {
  private val cpg = CpgLoader.load(TestCpgPaths.LLVM_SwitchesTestCPG)
  private val methodName = "switches";

  "types" in {
    validateTypes(cpg, List("ANY", "void", "void ()"))
  }

  "AST" in {
    val method = cpg.method.name(methodName).head
    val block = method.start.block.head
    block.start.astChildren.l.size shouldBe 5

    val switch = block.start.astChildren.head.asInstanceOf[Unknown]
    switch.code shouldBe "noop"

    val first = block.start.astChildren.l.apply(1).asInstanceOf[Unknown]
    first.code shouldBe "noop"

    val second = block.start.astChildren.l.apply(2).asInstanceOf[Unknown]
    second.code shouldBe "noop"

    val default = block.start.astChildren.l.apply(3).asInstanceOf[Unknown]
    default.code shouldBe "noop"

    val ret = block.start.astChildren.isReturn.head
    ret.code shouldBe "return"
  }

  "CFG" in {
    val method = cpg.method.name(methodName).head
    val block = method.start.block.head

    val switch = block.start.astChildren.head.asInstanceOf[Expression]
    val first = block.start.astChildren.l.apply(1).asInstanceOf[Expression]
    val second = block.start.astChildren.l.apply(2).asInstanceOf[Expression]
    val default = block.start.astChildren.l.apply(3).asInstanceOf[Expression]
    val ret = block.start.astChildren.isReturn.head

    method.start.cfgFirst.head shouldBe switch
    switch.start.cfgNext.toSet shouldBe Set(first, second, default)
    first.start.cfgNext.head shouldBe ret
    second.start.cfgNext.head shouldBe ret
    default.start.cfgNext.head shouldBe ret
  }
}
