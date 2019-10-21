package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._

/*
  %call = call i32 @dosomething(i32 45)
  ret void
 */
class C_CallIntTest extends CPGMatcher {
  private val cpg = CpgLoader.load(TestCpgPaths.C_CallIntCPG)
  private val methodName = "basic_c_support"

  "types" in {
    validateTypes(cpg, Set("ANY", "void", "i32"))
  }

  "AST" in {
    val method = cpg.method.name(methodName).head
    val block = method.start.block.head

    val callValue = block.start.local.head
    callValue.name shouldBe "call"
    callValue.typeFullName shouldBe "i32"

    val assignCall = block.start.astChildren.isCall.head
    val callValueRef = assignCall.start.astChildren.isIdentifier.head
    callValueRef.name shouldBe "call"
    callValueRef.typeFullName shouldBe "i32"
    callValueRef.start.refsTo.head shouldBe callValue

    val call = assignCall.start.astChildren.isCall.head
    call.name shouldBe "dosomething"
    call.typeFullName shouldBe "i32"

    val callParam = call.start.astChildren.isLiteral.head
    callParam.typeFullName shouldBe "i32"
    callParam.code shouldBe "45"
  }

  "Function declaration parameters" in {
    val method = cpg.method.name("dosomething").head
    method.start.parameter.l.size shouldBe 1
    val param = method.start.parameter.head
    param.name shouldBe "arg"
    param.typeFullName shouldBe "i32"
  }

  "CPG" in {
    val method = cpg.method.name(methodName).head
    val block = method.start.block.head

    val assignCall = block.start.astChildren.isCall.head
    val callValueRef = assignCall.start.astChildren.isIdentifier.head
    val call = assignCall.start.astChildren.isCall.head
    val callParam = call.start.astChildren.isLiteral.head

    callValueRef.start.cfgNext.head shouldBe callParam
    callParam.start.cfgNext.head shouldBe call
    call.start.cfgNext.head shouldBe assignCall

    val ret = block.start.astChildren.isReturnNode.head
    assignCall.start.cfgNext.head shouldBe ret
  }

}
