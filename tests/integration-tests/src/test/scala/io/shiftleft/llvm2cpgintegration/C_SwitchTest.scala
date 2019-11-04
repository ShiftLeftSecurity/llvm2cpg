package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._

class C_SwitchTest extends CPGMatcher {
  private val cpg = CpgLoader.load(TestCpgPaths.C_SwitchCPG)
  private val methodName = "basic_c_support"

  "types" in {
    validateTypes(cpg, Set("ANY", "i32", "i32*", "i32 (i32)"))
  }

  "CFG" in {
    /*
0:  %retval = alloca i32, align 4
1:  %x.addr = alloca i32, align 4
2:  store i32 %x, i32* %x.addr, align 4
3:  %tmp = load i32, i32* %x.addr, align 4
    switch i32 %0, label %sw.default [
      i32 0, label %sw.bb
      i32 1, label %sw.bb1
    ]

  sw.bb:                        ; preds = %entry
4:  store i32 42, i32* %retval
    br label %return

  sw.bb1:                       ; preds = %entry
5:  store i32 36, i32* %retval
    br label %return

  sw.default:                   ; preds = %entry
6:  store i32 17, i32* %retval
    br label %return

  return:                       ; preds = %sw.default, %sw.bb1, %sw.bb
7:  %1 = load i32, i32* %retval
    ret i32 %1
*/

    val method = cpg.method.name(methodName).head
    val block = method.start.block.head

    val calls = block.start.astChildren.isCall.l
    val loadParam = calls.apply(3)

    val assignStore42 = calls.apply(4)
    val assignStore42Lhs = assignStore42.start.astChildren.isCall.l.head
    val assignStore42Ref = assignStore42Lhs.start.astChildren.isIdentifier.l.head

    val assignStore36 = calls.apply(5)
    val assignStore36Lhs = assignStore36.start.astChildren.isCall.l.head
    val assignStore36Ref = assignStore36Lhs.start.astChildren.isIdentifier.l.head

    val assignStore17 = calls.apply(6)
    val assignStore17Lhs = assignStore17.start.astChildren.isCall.l.head
    val assignStore17Ref = assignStore17Lhs.start.astChildren.isIdentifier.l.head

    val loadRetVal = calls.apply(7)
    val loadRetValLhs =  loadRetVal.start.astChildren.isIdentifier.head

    loadParam.start.cfgNext.toSet shouldBe Set(assignStore42Ref, assignStore36Ref, assignStore17Ref)
    assignStore42.start.cfgNext.head shouldBe loadRetValLhs
    assignStore36.start.cfgNext.head shouldBe loadRetValLhs
    assignStore17.start.cfgNext.head shouldBe loadRetValLhs
  }

}
