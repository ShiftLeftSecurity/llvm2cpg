package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._
import overflowdb.traversal.NodeOps

/*
  %struct.Point = type { i32, i32 }

  %p = alloca %struct.Point                                           ; 0
  %x = getelementptr %struct.Point, %struct.Point* %p, i32 0, i32 0   ; 1
  store i32 127, i32* %x                                              ; 2
  %y = getelementptr %struct.Point, %struct.Point* %p, i32 0, i32 1   ; 3
  store i32 15, i32* %y                                               ; 4
  ret void                                                            ; 5
*/
class LLVM_GEPFlatStructTest extends CPGMatcher {
  private val cpg = CpgLoader.load(TestCpgPaths.LLVM_GEPFlatStructTestCPG)
  private val methodName = "flat_struct"

  "types" in {
    validateTypes(cpg, List("ANY", "Point", "Point*", "i32", "i32*", "void", "void ()"))
  }

  "locals" in {
    val method = cpg.method.name(methodName).head
    val block = method.start.block.head

    block.start.local.l.size shouldBe 3

    val p = block.start.local.head
    p.name shouldBe "p"
    p.typeFullName shouldBe "Point*"

    val x = block.start.local.l.apply(1)
    x.name shouldBe "x"
    x.typeFullName shouldBe "i32*"

    val y = block.start.local.l.apply(2)
    y.name shouldBe "y"
    y.typeFullName shouldBe "i32*"
  }

  "AST" in {
    val method = cpg.method.name(methodName).head
    val block = method.start.block.head
    val calls = block.start.astChildren.isCall.l

    val p = block.start.local.head
    val x = block.start.local.l.apply(1)
    val y = block.start.local.l.apply(2)

    // %x = getelementptr %struct.Point, %struct.Point* %p, i32 0, i32 0
    val assignGEP_X = calls.apply(0)

    val xRef = assignGEP_X.start.astChildren.isIdentifier.head
    xRef.name shouldBe "x"
    xRef.start.refsTo.head shouldBe x

    val pRef = assignGEP_X.start.ast.isIdentifier.l.apply(1)
    pRef.name shouldBe "p"
    pRef.start.refsTo.head shouldBe p

    // %y = getelementptr %struct.Point, %struct.Point* %p, i32 0, i32 0
    val assignGEP_Y = calls.apply(2)

    val yRef = assignGEP_Y.start.astChildren.isIdentifier.head
    yRef.name shouldBe "y"
    yRef.start.refsTo.head shouldBe y

    val memberAccessGEP_Y = assignGEP_Y.start.astChildren.isCall.head
    memberAccessGEP_Y.name shouldBe "<operator>.getElementPtr"

    val memberAccessGEP_Y_memberRef = memberAccessGEP_Y.start.argument(2).head
    memberAccessGEP_Y_memberRef.code shouldBe "1"

    val pointerShiftY = memberAccessGEP_Y.start.astChildren.isCall.head
    pointerShiftY.name shouldBe "<operator>.pointerShift"

    val pIdentifierY = pointerShiftY.start.astChildren.isIdentifier.head
    pIdentifierY.name shouldBe "p"
    pIdentifierY.start.refsTo.head shouldBe p
  }

  "CFG" in {
    val method = cpg.method.name(methodName).head
    val block = method.start.block.head
    val calls = block.start.astChildren.isCall.l

    // %x = getelementptr %struct.Point, %struct.Point* %p, i32 0, i32 0
    val assignGEP_X = calls.apply(0)
    val xRef = assignGEP_X.start.astChildren.isIdentifier.head
    val gep = assignGEP_X.start.astChildren.isCall.head
    val lit = gep.start.argument(2).l.head
    val pointerShift = gep.start.argument(1).isCall.l.head
    val pRef = pointerShift.start.argument(1).l.head
    val shiftLit = pointerShift.start.argument(2).l.head
    xRef.start.cfgNext.head shouldBe pRef
    pRef.start.cfgNext.head shouldBe shiftLit
    shiftLit.start.cfgNext.head shouldBe pointerShift
    pointerShift.start.cfgNext.head shouldBe lit
    lit.start.cfgNext.head shouldBe gep
    gep.start.cfgNext.head shouldBe assignGEP_X

    // store i32 127, i32* %x
    val assignStoreX = calls.apply(1)
    val lhsX = assignStoreX.start.astChildren.isCall.l.head
    val refX = lhsX.start.astChildren.isIdentifier.l.head
    val rhsX = assignStoreX.start.astChildren.isLiteral.l.head

    refX.start.cfgNext.head shouldBe lhsX
    lhsX.start.cfgNext.head shouldBe rhsX
    rhsX.start.cfgNext.head shouldBe assignStoreX

    assignGEP_X.start.cfgNext.head shouldBe refX

    // %y = getelementptr %struct.Point, %struct.Point* %p, i32 0, i32 1
    val assignGEP_Y = calls.apply(2)
    val yRef = assignGEP_Y.start.astChildren.isIdentifier.head
    val memberAccessGEP_Y = assignGEP_Y.start.astChildren.isCall.head
    val memberAccessGEP_Y_memberRef = memberAccessGEP_Y.start.argument(2).head
    val pointerShiftY = memberAccessGEP_Y.start.astChildren.isCall.head
    val pointerShiftYLit = pointerShiftY.start.astChildren.isLiteral.head
    val pIdentifierY = pointerShiftY.start.astChildren.isIdentifier.head

    assignStoreX.start.cfgNext.head shouldBe yRef
    yRef.start.cfgNext.head shouldBe pIdentifierY
    pIdentifierY.start.cfgNext.head shouldBe pointerShiftYLit
    pointerShiftYLit.start.cfgNext.head shouldBe pointerShiftY
    pointerShiftY.start.cfgNext.head shouldBe memberAccessGEP_Y_memberRef
    memberAccessGEP_Y_memberRef.start.cfgNext.head shouldBe memberAccessGEP_Y
    memberAccessGEP_Y.start.cfgNext.head shouldBe assignGEP_Y

    // store i32 15, i32* %y
    val assignStoreY = calls.apply(3)
    val lhsY = assignStoreY.start.astChildren.isCall.l.head
    val refY = lhsY.start.astChildren.isIdentifier.l.head
    val rhsY = assignStoreY.start.astChildren.isLiteral.l.head

    refY.start.cfgNext.head shouldBe lhsY
    lhsY.start.cfgNext.head shouldBe rhsY
    rhsY.start.cfgNext.head shouldBe assignStoreY

    assignGEP_Y.start.cfgNext.head shouldBe refY

    // ret void
    val ret = block.start.astChildren.isReturn.head
    assignStoreY.start.cfgNext.head shouldBe ret
  }

}
