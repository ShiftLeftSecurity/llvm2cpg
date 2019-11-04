package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.codepropertygraph.generated.nodes.{Expression, Unknown}
import io.shiftleft.semanticcpg.language._

class LLVM_BranchesTest extends CPGMatcher {
  private val cpg = CpgLoader.load(TestCpgPaths.LLVM_BranchesCPG)

  "types" in {
    validateTypes(cpg, Set("ANY", "void", "i8", "i1", "i8*"))
  }

  "empty branches AST" in {
    val method = cpg.method.name("empty_branches").head
    val block = method.start.block.head
    block.start.astChildren.l.size shouldBe 3

    val noop1 = block.start.astChildren.head.asInstanceOf[Unknown]
    noop1.code shouldBe "noop"

    val noop2 = block.start.astChildren.l.apply(2).asInstanceOf[Unknown]
    noop2.code shouldBe "noop"
    noop1.getId shouldNot be(noop2.getId)

    val ret = block.start.astChildren.isReturnNode.head
    ret.code shouldBe "return"
  }

  "empty branches CFG" in {
    val method = cpg.method.name("empty_branches").head
    val block = method.start.block.head

    val noop1 = block.start.astChildren.head.asInstanceOf[Expression]
    val noop2 = block.start.astChildren.l.apply(2).asInstanceOf[Expression]
    val ret = block.start.astChildren.isReturnNode.head

    method.start.cfgFirst.head shouldBe noop1
    noop1.start.cfgNext.head shouldBe noop2
    noop2.start.cfgNext.head shouldBe ret
  }

  "empty conditional branches AST" in {
    val method = cpg.method.name("empty_conditional_branches").head
    val block = method.start.block.head
    block.start.astChildren.l.size shouldBe 4

    val noop1 = block.start.astChildren.head.asInstanceOf[Unknown]
    noop1.code shouldBe "noop"

    val noop2 = block.start.astChildren.l.apply(1).asInstanceOf[Unknown]
    noop2.code shouldBe "noop"

    val noop3 = block.start.astChildren.l.apply(2).asInstanceOf[Unknown]
    noop3.code shouldBe "noop"

    noop1.getId shouldNot be(noop2.getId)
    noop2.getId shouldNot be(noop3.getId)
    noop3.getId shouldNot be(noop1.getId)

    val ret = block.start.astChildren.isReturnNode.head
    ret.code shouldBe "return"
  }

  "empty conditional branches CFG" in {
    val method = cpg.method.name("empty_conditional_branches").head
    val block = method.start.block.head

    val noop1 = block.start.astChildren.head.asInstanceOf[Expression]
    val noop2 = block.start.astChildren.l.apply(1).asInstanceOf[Expression]
    val noop3 = block.start.astChildren.l.apply(2).asInstanceOf[Expression]
    val ret = block.start.astChildren.isReturnNode.head

    method.start.cfgFirst.head shouldBe noop1
    noop1.start.cfgNext.toSet shouldBe Set(noop2, noop3)
    noop2.start.cfgNext.head shouldBe ret
    noop3.start.cfgNext.head shouldBe ret
  }

  "endless loop AST" in {
    val method = cpg.method.name("endless_loop").head
    val block = method.start.block.head
    block.start.astChildren.l.size shouldBe 3

    val noop1 = block.start.astChildren.head.asInstanceOf[Unknown]
    noop1.code shouldBe "noop"

    val noop2 = block.start.astChildren.l.apply(1).asInstanceOf[Unknown]
    noop2.code shouldBe "noop"
    noop1.getId shouldNot be(noop2.getId)

    val ret = block.start.astChildren.isReturnNode.head
    ret.code shouldBe "return"
  }

  "endless loop CFG" in {
    val method = cpg.method.name("endless_loop").head
    val block = method.start.block.head

    val noop1 = block.start.astChildren.head.asInstanceOf[Expression]
    val noop2 = block.start.astChildren.l.apply(1).asInstanceOf[Expression]
    val ret = block.start.astChildren.isReturnNode.head

    method.start.cfgFirst.head shouldBe noop1
    noop1.start.cfgNext.head shouldBe noop2
    noop2.start.cfgNext.head shouldBe noop2

    ret.start.cfgPrev.l.size shouldBe 0
  }

  "conditional CFG regression" in {
    val method = cpg.method.name("cfg_conditional").head
    val trunc = method.start.ast.isCall.name("<operator>.cast").head
    trunc.start.cfgNext.cfgNext.l.size shouldBe 1
    method.start.ast.isIdentifier.name("z").astParent.isCall.cfgNext.l.size shouldBe 2
  }

  "indirectbranch" in {
    val method = cpg.method.name("indirect_branch").head
    val assign_target = method.start.ast.isIdentifier.name("target").astParent.isCall.head
    assign_target.start.cfgNext.isLiteral.l.map{_.code}.toSet shouldBe Set[String]("1", "2")
  }
}
