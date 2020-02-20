package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._
import io.shiftleft.semanticcpg.language.types.expressions.MethodRef

/*
  declare i32 @something(...)

  %buf = alloca i8*
  %tmp = load i8*, i8** %buf
  %call = call i32 (i8*, ...) bitcast (i32 (...)* @something to i32 (i8*, ...)*)(i8* %tmp)
  ret void
*/
class C_CallUnknownFunctionTest extends CPGMatcher {
  private val cpg = CpgLoader.load(TestCpgPaths.C_CallUnknownFunctionTestCPG)
  private val methodName = "basic_c_support"

  "types" in {
    validateTypes(cpg, List("ANY", "i8*", "i8**", "i32", "void", "i32 (...)", "void ()"))
  }

  "AST" in {
    val method = cpg.method.name(methodName).head
    val block = method.start.block.head

    val tmpValue = block.start.local.l.apply(1)
    tmpValue.name shouldBe "tmp"
    tmpValue.typeFullName shouldBe "i8*"

    val callValue = block.start.local.l.apply(2)
    callValue.name shouldBe "call"
    callValue.typeFullName shouldBe "i32"

    val assignCall = block.start.astChildren.isCall.l.apply(1)
    val callValueRef = assignCall.start.astChildren.isIdentifier.head
    callValueRef.name shouldBe "call"
    callValueRef.typeFullName shouldBe "i32"
    callValueRef.start.refsTo.head shouldBe callValue

    val call = assignCall.start.astChildren.isCall.head
    call.name shouldBe "something"
    call.methodFullName shouldBe "something"
    call.typeFullName shouldBe "i32"
    call.dispatchType shouldBe "STATIC_DISPATCH"
    call.signature shouldBe "i32 (...)"
  }

  "CPG" in {
    val method = cpg.method.name(methodName).head
    val block = method.start.block.head

    val assignLoadCall = block.start.astChildren.isCall.l.apply(0)
    val assignCall = block.start.astChildren.isCall.l.apply(1)
    val callValueRef = assignCall.start.astChildren.isIdentifier.head
    val call = assignCall.start.astChildren.isCall.head
    val callParam = call.start.astChildren.isIdentifier.head
    val ret = block.start.astChildren.isReturn.head

    assignLoadCall.start.cfgNext.head shouldBe callValueRef
    callValueRef.start.cfgNext.head shouldBe callParam
    callParam.start.cfgNext.head shouldBe call
    call.start.cfgNext.head shouldBe assignCall
    assignCall.start.cfgNext.head shouldBe ret
  }

}
