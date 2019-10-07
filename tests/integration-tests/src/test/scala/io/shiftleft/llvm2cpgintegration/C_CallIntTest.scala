package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._
import org.scalatest.{Matchers, WordSpec}

/*
  %call = call i32 @dosomething(i32 45)
  ret void
 */
class C_CallIntTest extends WordSpec with Matchers {
  private val cpg = CpgLoader.load(TestCpgPaths.C_CallIntCPG)
  private val methodName = "basic_c_support"

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