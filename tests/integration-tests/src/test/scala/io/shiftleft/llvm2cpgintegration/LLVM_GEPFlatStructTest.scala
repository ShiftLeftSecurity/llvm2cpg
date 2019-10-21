package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._

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
  private val cpg = CpgLoader.load(TestCpgPaths.LLVM_GEPFlatStructCPG)
  private val methodName = "flat_struct"

  "types" in {
    validateTypes(cpg, Set("ANY", "struct.Point", "%struct.Point*", "i32", "i32*", "void"))
  }

  "locals" in {
    val method = cpg.method.name(methodName).head
    val block = method.start.block.head

    block.start.local.l.size shouldBe 3

    val p = block.start.local.head
    p.name shouldBe "p"
    p.typeFullName shouldBe "%struct.Point*"

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

    // %p = alloca %struct.Point
    val assignAlloca = calls.head
    assignAlloca.typeFullName shouldBe "%struct.Point*"
    val pRef = assignAlloca.start.astChildren.isIdentifier.head
    pRef.name shouldBe "p"
    pRef.start.refsTo.head shouldBe p

    // %x = getelementptr %struct.Point, %struct.Point* %p, i32 0, i32 0
    val assignGEP_X = calls.apply(1)

    val xRef = assignGEP_X.start.astChildren.isIdentifier.head
    xRef.name shouldBe "x"
    xRef.start.refsTo.head shouldBe x

    val memberAccessGEP_X = assignGEP_X.start.astChildren.isCall.head
    memberAccessGEP_X.name shouldBe "member_access"
    memberAccessGEP_X.typeFullName shouldBe "i32*"

    val memberAccessGEP_X_memberRef = memberAccessGEP_X.start.astChildren.isLiteral.head
    memberAccessGEP_X_memberRef.code shouldBe "0"

    val indexAccessGEP_X = memberAccessGEP_X.start.astChildren.isCall.head
    indexAccessGEP_X.name shouldBe "index_access"
    indexAccessGEP_X.typeFullName shouldBe "struct.Point"

    val indexAccessGEP_X_index = indexAccessGEP_X.start.astChildren.isLiteral.head
    indexAccessGEP_X_index.code shouldBe "0"

    val pIdentifierX = indexAccessGEP_X.start.astChildren.isIdentifier.head
    pIdentifierX.name shouldBe "p"
    pIdentifierX.start.refsTo.head shouldBe p

    // %y = getelementptr %struct.Point, %struct.Point* %p, i32 0, i32 0
    val assignGEP_Y = calls.apply(3)

    val yRef = assignGEP_Y.start.astChildren.isIdentifier.head
    yRef.name shouldBe "y"
    yRef.start.refsTo.head shouldBe y

    val memberAccessGEP_Y = assignGEP_Y.start.astChildren.isCall.head
    memberAccessGEP_Y.name shouldBe "member_access"

    val memberAccessGEP_Y_memberRef = memberAccessGEP_Y.start.astChildren.isLiteral.head
    memberAccessGEP_Y_memberRef.code shouldBe "1"

    val indexAccessGEP_Y = memberAccessGEP_Y.start.astChildren.isCall.head
    indexAccessGEP_Y.name shouldBe "index_access"

    val indexAccessGEP_Y_index = indexAccessGEP_Y.start.astChildren.isLiteral.head
    indexAccessGEP_Y_index.code shouldBe "0"

    val pIdentifierY = indexAccessGEP_Y.start.astChildren.isIdentifier.head
    pIdentifierY.name shouldBe "p"
    pIdentifierY.start.refsTo.head shouldBe p
  }

  "CFG" in {
    val method = cpg.method.name(methodName).head
    val block = method.start.block.head
    val calls = block.start.astChildren.isCall.l

    // %p = alloca %struct.Point
    val assignAlloca = calls.head

    // %x = getelementptr %struct.Point, %struct.Point* %p, i32 0, i32 0
    val assignGEP_X = calls.apply(1)

    val xRef = assignGEP_X.start.astChildren.isIdentifier.head
    val memberAccessGEP_X = assignGEP_X.start.astChildren.isCall.head
    val memberAccessGEP_X_memberRef = memberAccessGEP_X.start.astChildren.isLiteral.head
    val indexAccessGEP_X = memberAccessGEP_X.start.astChildren.isCall.head
    val indexAccessGEP_X_index = indexAccessGEP_X.start.astChildren.isLiteral.head
    val pIdentifierX = indexAccessGEP_X.start.astChildren.isIdentifier.head

    assignAlloca.start.cfgNext.head shouldBe xRef
    xRef.start.cfgNext.head shouldBe pIdentifierX
    pIdentifierX.start.cfgNext.head shouldBe indexAccessGEP_X_index
    indexAccessGEP_X_index.start.cfgNext.head shouldBe indexAccessGEP_X
    indexAccessGEP_X.start.cfgNext.head shouldBe memberAccessGEP_X_memberRef
    memberAccessGEP_X_memberRef.start.cfgNext.head shouldBe memberAccessGEP_X
    memberAccessGEP_X.start.cfgNext.head shouldBe assignGEP_X

    // store i32 127, i32* %x
    val assignStoreX = calls.apply(2)
    val lhsX = assignStoreX.start.astChildren.isCall.l.head
    val refX = lhsX.start.astChildren.isIdentifier.l.head
    val rhsX = assignStoreX.start.astChildren.isLiteral.l.head

    refX.start.cfgNext.head shouldBe lhsX
    lhsX.start.cfgNext.head shouldBe rhsX
    rhsX.start.cfgNext.head shouldBe assignStoreX

    assignGEP_X.start.cfgNext.head shouldBe refX

    // %y = getelementptr %struct.Point, %struct.Point* %p, i32 0, i32 1
    val assignGEP_Y = calls.apply(3)
    val yRef = assignGEP_Y.start.astChildren.isIdentifier.head
    val memberAccessGEP_Y = assignGEP_Y.start.astChildren.isCall.head
    val memberAccessGEP_Y_memberRef = memberAccessGEP_Y.start.astChildren.isLiteral.head
    val indexAccessGEP_Y = memberAccessGEP_Y.start.astChildren.isCall.head
    val indexAccessGEP_Y_index = indexAccessGEP_Y.start.astChildren.isLiteral.head
    val pIdentifierY = indexAccessGEP_Y.start.astChildren.isIdentifier.head

    assignStoreX.start.cfgNext.head shouldBe yRef
    yRef.start.cfgNext.head shouldBe pIdentifierY
    pIdentifierY.start.cfgNext.head shouldBe indexAccessGEP_Y_index
    indexAccessGEP_Y_index.start.cfgNext.head shouldBe indexAccessGEP_Y
    indexAccessGEP_Y.start.cfgNext.head shouldBe memberAccessGEP_Y_memberRef
    memberAccessGEP_Y_memberRef.start.cfgNext.head shouldBe memberAccessGEP_Y
    memberAccessGEP_Y.start.cfgNext.head shouldBe assignGEP_Y

    // store i32 15, i32* %y
    val assignStoreY = calls.apply(4)
    val lhsY = assignStoreY.start.astChildren.isCall.l.head
    val refY = lhsY.start.astChildren.isIdentifier.l.head
    val rhsY = assignStoreY.start.astChildren.isLiteral.l.head

    refY.start.cfgNext.head shouldBe lhsY
    lhsY.start.cfgNext.head shouldBe rhsY
    rhsY.start.cfgNext.head shouldBe assignStoreY

    assignGEP_Y.start.cfgNext.head shouldBe refY

    // ret void
    val ret = block.start.astChildren.isReturnNode.head
    assignStoreY.start.cfgNext.head shouldBe ret
  }

}
