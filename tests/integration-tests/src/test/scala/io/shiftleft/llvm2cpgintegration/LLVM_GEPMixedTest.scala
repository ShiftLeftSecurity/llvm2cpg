package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._

/*
  %struct.RT = type { i8, [10 x [20 x i32]], i8 }
  %struct.ST = type { i32, double, %struct.RT }

  %index = getelementptr %struct.ST, %struct.ST* %s, i64 1, i32 2, i32 1, i64 5, i64 13 ; 1
  ret i32* %index                                                                       ; 2
*/

class LLVM_GEPMixedTest extends CPGMatcher {
  private val cpg = CpgLoader.load(TestCpgPaths.LLVM_GEPMixedCPG)
  private val methodName = "gep_mixed"

  "types" in {
    validateTypes(cpg, Set("ANY", "%struct.ST*", "struct.ST", "struct.RT", "i32", "i32*", "i64", "[20 x i32]", "[10 x [20 x i32]]", "i32* (%struct.ST*)"))
  }

  "AST" in {
    val method = cpg.method.name(methodName).head
    val block = method.start.block.head

    val index = block.start.local.head
    val s = method.start.parameter.head

    // %index = getelementptr %struct.ST, %struct.ST* %s, i64 4, i32 2, i32 1, i64 5, i64 13
    val assignGEP = block.start.astChildren.isCall.head

    val indexRef = assignGEP.start.astChildren.isIdentifier.head
    indexRef.name shouldBe "index"
    indexRef.start.refsTo.head shouldBe index

    // i64 13
    val indexAccessGEP_13 = assignGEP.start.astChildren.isCall.head
    indexAccessGEP_13.name shouldBe "<operator>.computedMemberAccess"
    indexAccessGEP_13.typeFullName shouldBe "i32*"
    val indexAccessGEP_13_index = indexAccessGEP_13.start.astChildren.isLiteral.head
    indexAccessGEP_13_index.code shouldBe "13"

    // i64 5
    val indexAccessGEP_5 = indexAccessGEP_13.start.astChildren.isCall.head
    indexAccessGEP_5.name shouldBe "<operator>.computedMemberAccess"
    indexAccessGEP_5.typeFullName shouldBe "[20 x i32]"
    val indexAccessGEP_5_index = indexAccessGEP_5.start.astChildren.isLiteral.head
    indexAccessGEP_5_index.code shouldBe "5"

    // i32 1
    val indexAccessGEP_1 = indexAccessGEP_5.start.astChildren.isCall.head
    indexAccessGEP_1.name shouldBe "<operator>.memberAccess"
    indexAccessGEP_1.typeFullName shouldBe "[10 x [20 x i32]]"
    val indexAccessGEP_1_index = indexAccessGEP_1.start.astChildren.isLiteral.head
    indexAccessGEP_1_index.code shouldBe "1"

    // i32 2
    val indexAccessGEP_2 = indexAccessGEP_1.start.astChildren.isCall.head
    indexAccessGEP_2.name shouldBe "<operator>.memberAccess"
    indexAccessGEP_2.typeFullName shouldBe "struct.RT"
    val indexAccessGEP_2_index = indexAccessGEP_2.start.astChildren.isLiteral.head
    indexAccessGEP_2_index.code shouldBe "2"

    // i64 4
    val indexAccessGEP_4 = indexAccessGEP_2.start.astChildren.isCall.head
    indexAccessGEP_4.name shouldBe "<operator>.computedMemberAccess"
    indexAccessGEP_4.typeFullName shouldBe "struct.ST"
    val indexAccessGEP_4_index = indexAccessGEP_4.start.astChildren.isLiteral.head
    indexAccessGEP_4_index.code shouldBe "4"

    val sIdentifier = indexAccessGEP_4.start.astChildren.isIdentifier.head
    sIdentifier.name shouldBe "s"
    sIdentifier.start.refsTo.head shouldBe s
  }

  "CFG" in {
    val method = cpg.method.name(methodName).head
    val block = method.start.block.head

    // %index = getelementptr %struct.ST, %struct.ST* %s, i64 4, i32 2, i32 1, i64 5, i64 13
    val assignGEP = block.start.astChildren.isCall.head

    val indexRef = assignGEP.start.astChildren.isIdentifier.head

    // i64 13
    val indexAccessGEP_13 = assignGEP.start.astChildren.isCall.head
    val indexAccessGEP_13_index = indexAccessGEP_13.start.astChildren.isLiteral.head

    // i64 5
    val indexAccessGEP_5 = indexAccessGEP_13.start.astChildren.isCall.head
    val indexAccessGEP_5_index = indexAccessGEP_5.start.astChildren.isLiteral.head

    // i32 1
    val indexAccessGEP_1 = indexAccessGEP_5.start.astChildren.isCall.head
    val indexAccessGEP_1_index = indexAccessGEP_1.start.astChildren.isLiteral.head

    // i32 2
    val indexAccessGEP_2 = indexAccessGEP_1.start.astChildren.isCall.head
    val indexAccessGEP_2_index = indexAccessGEP_2.start.astChildren.isLiteral.head

    // i64 4
    val indexAccessGEP_4 = indexAccessGEP_2.start.astChildren.isCall.head
    val indexAccessGEP_4_index = indexAccessGEP_4.start.astChildren.isLiteral.head

    val sIdentifier = indexAccessGEP_4.start.astChildren.isIdentifier.head

    indexRef.start.cfgNext.head shouldBe sIdentifier
    sIdentifier.start.cfgNext.head shouldBe indexAccessGEP_4_index
    indexAccessGEP_4_index.start.cfgNext.head shouldBe indexAccessGEP_4
    indexAccessGEP_4.start.cfgNext.head shouldBe indexAccessGEP_2_index
    indexAccessGEP_2_index.start.cfgNext.head shouldBe indexAccessGEP_2
    indexAccessGEP_2.start.cfgNext.head shouldBe indexAccessGEP_1_index
    indexAccessGEP_1_index.start.cfgNext.head shouldBe indexAccessGEP_1
    indexAccessGEP_1.start.cfgNext.head shouldBe indexAccessGEP_5_index
    indexAccessGEP_5_index.start.cfgNext.head shouldBe indexAccessGEP_5
    indexAccessGEP_5.start.cfgNext.head shouldBe indexAccessGEP_13_index
    indexAccessGEP_13_index.start.cfgNext.head shouldBe indexAccessGEP_13
    indexAccessGEP_13.start.cfgNext.head shouldBe assignGEP

    // ret i32* %index
    val ret = block.start.astChildren.isReturnNode.head
    val retPtr = ret.start.astChildren.isIdentifier.head
    assignGEP.start.cfgNext.head shouldBe retPtr
  }

}
