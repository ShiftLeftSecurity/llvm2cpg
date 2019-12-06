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
  private val cpg = CpgLoader.load(TestCpgPaths.C_CallUnknownFunctionCPG)
  private val methodName = "basic_c_support"

  "types" in {
    validateTypes(cpg, Set("ANY", "i8*", "i8**", "i32", "void", "i32 (i8*, ...)*", "i32 (...)", "void ()"))
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

    val ptrCall = assignCall.start.astChildren.isCall.head
    ptrCall.name shouldBe "indirect_call"
    ptrCall.methodFullName shouldBe "indirect_call"
    ptrCall.typeFullName shouldBe "i32"
    ptrCall.dispatchType shouldBe "DYNAMIC_DISPATCH"
    ptrCall.signature shouldBe "i32 (i8*, ...)*"

    val ptrCallParam = ptrCall.start.astChildren.isIdentifier.head
    ptrCallParam.typeFullName shouldBe "i8*"
    ptrCallParam.name shouldBe "tmp"
    ptrCallParam.start.refsTo.head shouldBe tmpValue

    val receiver = ptrCall.start.receiver.isCall.head
    receiver.name shouldBe "<operator>.cast"
    receiver.typeFullName shouldBe "i32 (i8*, ...)*"
    val methodRef = receiver.start.astChildren.isMethodRef.head
    methodRef.methodFullName shouldBe "something"

    // TODO: Receiver should not be connected via AST edge
    // But it is for now
    // ptrCall.start.astChildren.l.size shouldBe 1
    val receiverAST = ptrCall.start.astChildren.isCall.head
    receiver shouldBe receiverAST
  }

  "CPG" in {
    val method = cpg.method.name(methodName).head
    val block = method.start.block.head

    val assignLoadCall = block.start.astChildren.isCall.l.apply(0)
    val assignCall = block.start.astChildren.isCall.l.apply(1)
    val callValueRef = assignCall.start.astChildren.isIdentifier.head
    val ptrCall = assignCall.start.astChildren.isCall.head
    val ptrCallParam = ptrCall.start.astChildren.isIdentifier.head
    val receiver = ptrCall.start.receiver.isCall.head
    val methodRef = receiver.start.astChildren.isMethodRef.head
    val ret = block.start.astChildren.isReturnNode.head

    assignLoadCall.start.cfgNext.head shouldBe callValueRef
    callValueRef.start.cfgNext.head shouldBe methodRef
    new MethodRef(methodRef.start.raw).cfgNext.head shouldBe receiver
    receiver.start.cfgNext.head shouldBe ptrCallParam
    ptrCallParam.start.cfgNext.head shouldBe ptrCall
    ptrCall.start.cfgNext.head shouldBe assignCall
    assignCall.start.cfgNext.head shouldBe ret
  }

}
