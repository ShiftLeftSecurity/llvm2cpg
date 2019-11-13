package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._

class LLVM_SelectInstTest extends CPGMatcher {
  private val cpg = CpgLoader.load(TestCpgPaths.LLVM_SelectInstCPG)
  private val methodName = "basic_c_support"

  "types" in {
    validateTypes(cpg, Set("ANY", "i32", "i1", "i32 ()"))
  }

  "AST" in {
  /*
    %sel = select i1 true, i32 15, i32 17
    ret i32 %sel
  */

    val method = cpg.method.name(methodName).head
    val block = method.start.block.head

    val sel = block.start.local.head
    sel.name shouldBe "sel"

    block.start.astChildren.isReturnNode.l.size shouldBe 1
    block.start.astChildren.isCall.l.size shouldBe 1

    val assignSelect = block.start.astChildren.isCall.head
    val selRef = assignSelect.start.astChildren.isIdentifier.head
    selRef.name shouldBe "sel"
    selRef.start.refsTo.head shouldBe sel

    val selectCall = assignSelect.start.astChildren.isCall.head
    selectCall.name shouldBe "<operator>.select"
    selectCall.methodFullName shouldBe "<operator>.select"
    selectCall.signature shouldBe "ANY (ANY, ANY, ANY)"

    val selectCondition = selectCall.start.astChildren.isLiteral.l.head
    selectCondition.code shouldBe "true"
    selectCondition.typeFullName shouldBe "i1"

    val selectTrue = selectCall.start.astChildren.isLiteral.l.apply(1)
    selectTrue.code shouldBe "15"
    selectTrue.typeFullName shouldBe "i32"

    val selectFalse = selectCall.start.astChildren.isLiteral.l.apply(2)
    selectFalse.code shouldBe "17"
    selectFalse.typeFullName shouldBe "i32"

    val ret = block.start.astChildren.isReturnNode.head
    val retRef = ret.start.astChildren.isIdentifier.head
    retRef.name shouldBe "sel"
    retRef.start.refsTo.head shouldBe sel
  }

  "CFG" in {
  /*
    %sel = select i1 true, i32 15, i32 17
    ret i32 %sel
  */
    val method = cpg.method.name(methodName).head
    val block = method.start.block.head

    val assignSelect = block.start.astChildren.isCall.head
    val selRef = assignSelect.start.astChildren.isIdentifier.head
    val selectCall = assignSelect.start.astChildren.isCall.head
    val selectCondition = selectCall.start.astChildren.isLiteral.l.head
    val selectTrue = selectCall.start.astChildren.isLiteral.l.apply(1)
    val selectFalse = selectCall.start.astChildren.isLiteral.l.apply(2)
    val ret = block.start.astChildren.isReturnNode.head
    val retRef = ret.start.astChildren.isIdentifier.head

    method.start.cfgFirst.head shouldBe selRef
    selRef.start.cfgNext.head shouldBe selectCondition
    selectCondition.start.cfgNext.head shouldBe selectTrue
    selectTrue.start.cfgNext.head shouldBe selectFalse
    selectFalse.start.cfgNext.head shouldBe selectCall
    selectCall.start.cfgNext.head shouldBe assignSelect
    assignSelect.start.cfgNext.head shouldBe retRef
    retRef.start.cfgNext.head shouldBe ret
    method.start.methodReturn.cfgLast.head shouldBe ret
    // TODO: the line below should also work, right?
//    ret.start.cfgNext.head shouldBe method.start.methodReturn
  }

}
