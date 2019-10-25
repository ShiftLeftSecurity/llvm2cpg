package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._

/*
  @.str = private unnamed_addr constant [6 x i8] c"hello\00"

  %call = call i32 @printstuff(i8* getelementptr ([6 x i8], [6 x i8]* @.str, i32 0, i32 0))
  ret void
*/
class C_CallStringTest extends CPGMatcher {
  private val cpg = CpgLoader.load(TestCpgPaths.C_CallStringCPG)
  private val methodName = "basic_c_support"

  "types" in {
    validateTypes(cpg, Set("ANY", "i8*", "void", "i32", "[6 x i8]", "[6 x i8]*"))
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

    val indexAccess_0 = call.start.astChildren.isCall.head
    indexAccess_0.typeFullName shouldBe "i8*"
    indexAccess_0.name shouldBe "index_access"
    indexAccess_0.methodFullName shouldBe "index_access"

    val indexAccess_0_index = indexAccess_0.start.astChildren.isLiteral.head
    indexAccess_0_index.code shouldBe "0"
    indexAccess_0_index.typeFullName shouldBe "i32"

    val indexAccess_1 = indexAccess_0.start.astChildren.isCall.head
    indexAccess_1.typeFullName shouldBe "[6 x i8]"
    indexAccess_1.name shouldBe "index_access"

    val indexAccess_1_ref = indexAccess_1.start.astChildren.isIdentifier.head
    indexAccess_1_ref.typeFullName shouldBe "[6 x i8]*"
    indexAccess_1_ref.name shouldBe ".str"
    val indexAccess_1_index = indexAccess_1.start.astChildren.isLiteral.head
    indexAccess_1_index.typeFullName shouldBe "i32"
    indexAccess_1_index.code shouldBe "0"
  }

  "CPG" in {
    val method = cpg.method.name(methodName).head
    val block = method.start.block.head

    val assignCall = block.start.astChildren.isCall.head
    val callValueRef = assignCall.start.astChildren.isIdentifier.head
    val call = assignCall.start.astChildren.isCall.head
    val indexAccess_0 = call.start.astChildren.isCall.head
    val indexAccess_0_index = indexAccess_0.start.astChildren.isLiteral.head
    val indexAccess_1 = indexAccess_0.start.astChildren.isCall.head
    val indexAccess_1_ref = indexAccess_1.start.astChildren.isIdentifier.head
    val indexAccess_1_index = indexAccess_1.start.astChildren.isLiteral.head

    callValueRef.start.cfgNext.head shouldBe indexAccess_1_ref
    indexAccess_1_ref.start.cfgNext.head shouldBe indexAccess_1_index
    indexAccess_1_index.start.cfgNext.head shouldBe indexAccess_1
    indexAccess_1.start.cfgNext.head shouldBe indexAccess_0_index
    indexAccess_0_index.start.cfgNext.head shouldBe indexAccess_0
    indexAccess_0.start.cfgNext.head shouldBe call
    call.start.cfgNext.head shouldBe assignCall

    val ret = block.start.astChildren.isReturnNode.head
    assignCall.start.cfgNext.head shouldBe ret
  }

}
