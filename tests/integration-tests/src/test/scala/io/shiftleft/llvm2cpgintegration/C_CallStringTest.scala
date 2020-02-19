package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._

/*
  @.str = private unnamed_addr constant [6 x i8] c"hello\00"

  %call = call i32 @printstuff(i8* getelementptr ([6 x i8], [6 x i8]* @.str, i32 0, i32 0))
  ret void
*/
class C_CallStringTest extends CPGMatcher {
  private val cpg = CpgLoader.load(TestCpgPaths.C_CallStringTestCPG)
  private val methodName = "basic_c_support"

  "types" in {
    validateTypes(cpg, Set("ANY", "i8*", "void", "i32", "[6 x i8]", "i32 (i8*)", "void ()"))
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
    call.name shouldBe "printstuff"
    call.methodFullName shouldBe "printstuff"
    call.typeFullName shouldBe "i32"
    call.signature shouldBe "i32 (i8*)"

    val addressOf = call.start.astChildren.isCall.head
    addressOf.name shouldBe "<operator>.addressOf"
    addressOf.code shouldBe "addressOf"
    addressOf.typeFullName shouldBe "ANY"

    val argument = addressOf.start.astChildren.isLiteral.head
    argument.code shouldBe "hello"
    argument.typeFullName shouldBe "[6 x i8]"
  }

  "CPG" in {
    val method = cpg.method.name(methodName).head
    val block = method.start.block.head

    val assignCall = block.start.astChildren.isCall.head
    val callValueRef = assignCall.start.astChildren.isIdentifier.head
    val call = assignCall.start.astChildren.isCall.head
    val addressOf = call.start.astChildren.isCall.head
    val argument = addressOf.start.astChildren.isLiteral.head

    callValueRef.start.cfgNext.head shouldBe argument
    argument.start.cfgNext.head shouldBe addressOf
    addressOf.start.cfgNext.head shouldBe call
    call.start.cfgNext.head shouldBe assignCall

    val ret = block.start.astChildren.isReturn.head
    assignCall.start.cfgNext.head shouldBe ret
  }

}
