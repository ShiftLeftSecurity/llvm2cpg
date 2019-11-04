package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._

/*
Original code:

entry:
  br i1 true, label %cond.true, label %cond.false

cond.true:                                        ; preds = %entry
  %call = call i32 @foo()
  br label %cond.end

cond.false:                                       ; preds = %entry
  %call1 = call i32 @foo()
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond = phi i32 [ %call, %cond.true ], [ %call1, %cond.false ]
  ret i32 %cond

 */

/*

Transformed code (no phi-nodes):

entry:
 %cond.reg2mem = alloca i32                       ; 0
 br i1 true, label %cond.true, label %cond.false

cond.true:                                        ; preds = %entry
 %call = call i32 @foo()                          ; 1
 store i32 %call, i32* %cond.reg2mem              ; 2
 br label %cond.end

cond.false:                                       ; preds = %entry
 %call1 = call i32 @bar()                         ; 3
 store i32 %call1, i32* %cond.reg2mem             ; 4
 br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
 %cond.reload = load i32, i32* %cond.reg2mem      ; 5
 ret i32 %cond.reload                             ; 6
*/

class LLVM_PHINodeTest extends CPGMatcher {
  private val cpg = CpgLoader.load(TestCpgPaths.LLVM_PHINodeCPG)
  private val methodName = "foobar"

  "types" in {
    validateTypes(cpg, Set("ANY", "i32", "i32*", "i32 ()"))
  }

  "AST" in {
    val method = cpg.method.name(methodName).head
    val block = method.start.block.head

    val condReg2Mem = block.start.local.head
    condReg2Mem.name shouldBe "cond.reg2mem"

    val call = block.start.local.l.apply(1)
    call.name shouldBe "call"

    val call1 = block.start.local.l.apply(2)
    call1.name shouldBe "call1"

    val condReload = block.start.local.l.apply(3)
    condReload.name shouldBe "cond.reload"

    val assignAlloca = block.start.astChildren.isCall.head
    val allocaCall = assignAlloca.start.astChildren.isCall.head
    allocaCall.name shouldBe "<operator>.alloca"

    val store1 = block.start.astChildren.isCall.l.apply(2)
    val store1_ref = store1.start.astChildren.isIdentifier.head
    store1_ref.name shouldBe "call"
    store1_ref.start.refsTo.head shouldBe call
    val store1_indirection = store1.start.astChildren.isCall.head
    val store1_indirectionRef = store1_indirection.start.astChildren.isIdentifier.head
    store1_indirectionRef.name shouldBe "cond.reg2mem"
    store1_indirectionRef.start.refsTo.head shouldBe condReg2Mem

    val store2 = block.start.astChildren.isCall.l.apply(4)
    val store2_ref = store2.start.astChildren.isIdentifier.head
    store2_ref.name shouldBe "call1"
    store2_ref.start.refsTo.head shouldBe call1
    val store2_indirection = store2.start.astChildren.isCall.head
    val store2_indirectionRef = store2_indirection.start.astChildren.isIdentifier.head
    store2_indirectionRef.name shouldBe "cond.reg2mem"
    store2_indirectionRef.start.refsTo.head shouldBe condReg2Mem

    val load = block.start.astChildren.isCall.l.last
    val loadRef = load.start.astChildren.isIdentifier.head
    loadRef.name shouldBe "cond.reload"
    loadRef.start.refsTo.head shouldBe condReload
    val loadIndirection = load.start.astChildren.isCall.head
    val loadIndirectionRef = loadIndirection.start.astChildren.isIdentifier.head
    loadIndirectionRef.name shouldBe "cond.reg2mem"
    loadIndirectionRef.start.refsTo.head shouldBe condReg2Mem
  }

}
