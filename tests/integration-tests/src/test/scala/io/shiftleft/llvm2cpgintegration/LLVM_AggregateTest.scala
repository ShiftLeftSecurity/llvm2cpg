package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._
import org.scalatest.{Matchers, WordSpec}
import io.shiftleft.codepropertygraph.generated.nodes._
import io.shiftleft.semanticcpg.language.types.expressions.generalizations

class LLVM_AggregateTest extends CPGMatcher {
    private val cpg = CpgLoader.load(TestCpgPaths.LLVM_AggregateTestCpg)

    "types" in {
        validateTypes(cpg, Set(
            "{ i8, i8, i8, [4 x i8] }",
            "ANY",
            "float",
            "[1 x float]",
            "i8",
            "{ i32, { [3 x { i8, i8, i8, [4 x i8] }] } }",
            "[4 x i8]",
            "{ i32, { [1 x float] } }",
            "{ [1 x float] }",
            "[3 x { i8, i8, i8, [4 x i8] }]",
            "{ [3 x { i8, i8, i8, [4 x i8] }] }",
            "i32",
            "{ i32, float }",
            "void"
            )
        )
    }

    "CFG" in {
        val extractBParent = cpg.method.name("extract").ast.isIdentifier.name("B").astParent.isCall.l.head
        extractBParent.name shouldBe "<operator>.assignment"
        val P1 = extractBParent.start.cfgPrev.isCall.head
        P1.code shouldBe "ExtractValue_index"
        P1.typeFullName shouldBe "i8"
        val P2 = P1.start.cfgPrev.isLiteral.head
        P2.code shouldBe "0"
        val P3 = P2.start.cfgPrev.isCall.head
        P3.code shouldBe "ExtractValue_member"
        P3.typeFullName shouldBe "[4 x i8]"
        val P4 = P3.start.cfgPrev.isLiteral.head
        P4.code shouldBe "3"
    }

    "AST" in {
        val extract_B = cpg.method.name("extract").ast.isIdentifier.name("B").astParent.head
        treeDump(extract_B) shouldBe (
            "CALL",
            "<operator>.assignment",
            "i8",
            List(
                (1, ("IDENTIFIER", "B", "i8")),
                (
                2,
                (
                    "CALL",
                    "ExtractValue_index",
                    "i8",
                    List(
                    (
                        1,
                        (
                        "CALL",
                        "ExtractValue_member",
                        "[4 x i8]",
                        List(
                            (
                            1,
                            (
                                "CALL",
                                "ExtractValue_index",
                                "{ i8, i8, i8, [4 x i8] }",
                                List(
                                (
                                    1,
                                    (
                                    "CALL",
                                    "ExtractValue_member",
                                    "[3 x { i8, i8, i8, [4 x i8] }]",
                                    List(
                                        (
                                        1,
                                        (
                                            "CALL",
                                            "ExtractValue_member",
                                            "{ [3 x { i8, i8, i8, [4 x i8] }] }",
                                            List((1, ("LITERAL", "zero initialized", "{ i32, { [3 x { i8, i8, i8, [4 x i8] }] } }")), (2, ("LITERAL", "1", "i32")))
                                        )
                                        ),
                                        (2, ("LITERAL", "0", "i32"))
                                    )
                                    )
                                ),
                                (2, ("LITERAL", "2", "i32"))
                                )
                            )
                            ),
                            (2, ("LITERAL", "3", "i32"))
                        )
                        )
                    ),
                    (2, ("LITERAL", "0", "i32"))
                    )
                )
                )
            )
        )
        val insert_agg3 = cpg.method.name("insert").ast.isIdentifier.name("agg3").astParent.head
        treeDump(insert_agg3) shouldBe (
            "CALL",
            "<operator>.assignment",
            "{ i32, { [1 x float] } }",
            List(
                (1, ("IDENTIFIER", "agg3", "{ i32, { [1 x float] } }")),
                (
                2,
                (
                    "CALL",
                    "insertValue",
                    "{ i32, { [1 x float] } }",
                    List(
                    (
                        1,
                        (
                        "CALL",
                        "insertValue_indexShift",
                        "float",
                        List(
                            (
                            1,
                            (
                                "CALL",
                                "insertValue_memberSelect",
                                "[1 x float]",
                                List(
                                (
                                    1,
                                    ("CALL", "insertValue_memberSelect", "{ [1 x float] }", List((1, ("LITERAL", "undef", "{ i32, { [1 x float] } }")), (2, ("LITERAL", "1", "i32"))))
                                ),
                                (2, ("LITERAL", "0", "i32"))
                                )
                            )
                            ),
                            (2, ("LITERAL", "0", "i32"))
                        )
                        )
                    ),
                    (2, ("IDENTIFIER", "val", "float"))
                    )
                )
                )
            )
        )
    } // end AST
}