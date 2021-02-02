package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._
import org.scalatest.{Matchers, WordSpec}
import io.shiftleft.codepropertygraph.generated.nodes._
import io.shiftleft.semanticcpg.language.types.expressions.generalizations
import overflowdb.traversal.NodeOps

class LLVM_AggregateTest extends CPGMatcher {
    private val cpg = CpgLoader.load(TestCpgPaths.LLVM_AggregateTestCPG)

    "types" in {
        validateTypes(cpg, List(
            "{ i8, i8, i8, [4 x i8] }",
            "ANY",
            "float",
//            "[1 x float]",
            "i8",
            "{ i32, { [3 x { i8, i8, i8, [4 x i8] }] } }",
            "[4 x i8]",
            "{ i32, { [1 x float] } }",
//            "{ [1 x float] }",
            "[3 x { i8, i8, i8, [4 x i8] }]",
            "{ [3 x { i8, i8, i8, [4 x i8] }] }",
            "i32",
            "{ i32, float }",
            "void",
            "void ()",
            "{ i32, float } (float)"
            )
        )
    }

    "CFG" in {
        val extractBParent = cpg.method.name("extract").ast.isIdentifier.name("B").astParent.isCall.l.head
        extractBParent.name shouldBe "<operator>.assignment"
        val P1 = extractBParent.start.cfgPrev.isCall.head
        P1.code shouldBe "extractValue()"
        P1.typeFullName shouldBe "i8"
        val P2 = P1.start.cfgPrev.isLiteral.head
        P2.code shouldBe "0"
        val P3 = P2.start.cfgPrev.isCall.head
        P3.code shouldBe "extractValue()"
        P3.typeFullName shouldBe "[4 x i8]"
        val P4 = P3.start.cfgPrev.isLiteral.head
        P4.code shouldBe "3"
    }

    "AST" in {
        val extract_B = cpg.method.name("extract").ast.isIdentifier.name("B").astParent.head
      treeDump(extract_B, new StringBuilder()).toString() shouldBe
      """CALL name = <operator>.assignment fullname = <operator>.assignment return type = i8
         |	IDENTIFIER code = B type = i8
         |	CALL name = <operator>.indexAccess fullname = <operator>.indexAccess return type = i8
         |		CALL name = <operator>.indexAccess fullname = <operator>.indexAccess return type = [4 x i8]
         |			CALL name = <operator>.indexAccess fullname = <operator>.indexAccess return type = { i8, i8, i8, [4 x i8] }
         |				CALL name = <operator>.indexAccess fullname = <operator>.indexAccess return type = [3 x { i8, i8, i8, [4 x i8] }]
         |					CALL name = <operator>.indexAccess fullname = <operator>.indexAccess return type = { [3 x { i8, i8, i8, [4 x i8] }] }
         |						LITERAL code = zero initialized type = { i32, { [3 x { i8, i8, i8, [4 x i8] }] } }
         |						LITERAL code = 1 type = i32
         |					LITERAL code = 0 type = i32
         |				LITERAL code = 2 type = i32
         |			LITERAL code = 3 type = i32
         |		LITERAL code = 0 type = i32
         |""".stripMargin



        val insert_agg3 = cpg.method.name("insert").ast.isIdentifier.name("agg3").astParent.head
      treeDump(insert_agg3, new StringBuilder()).toString() shouldBe
      """CALL name = <operator>.assignment fullname = <operator>.assignment return type = { i32, { [1 x float] } }
         |	IDENTIFIER code = agg3 type = { i32, { [1 x float] } }
         |	CALL name = <operator>.insertValue fullname = <operator>.insertValue return type = { i32, { [1 x float] } }
         |		LITERAL code = undef type = { i32, { [1 x float] } }
         |		IDENTIFIER code = val type = float
         |		LITERAL code = 1 type = i32
         |		LITERAL code = 0 type = i32
         |		LITERAL code = 0 type = i32
       |""".stripMargin

    } // end AST
}