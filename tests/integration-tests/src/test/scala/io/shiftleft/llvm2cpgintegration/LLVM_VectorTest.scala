package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._
import org.scalatest.{Matchers, WordSpec}
import io.shiftleft.codepropertygraph.generated.nodes._
import io.shiftleft.semanticcpg.language.types.expressions.generalizations
import overflowdb.traversal.NodeOps

class LLVM_VectorTest extends CPGMatcher {
  private val cpg = CpgLoader.load(TestCpgPaths.LLVM_VectorTestCPG)

  "types" in {
    validateTypes(cpg, List(
        "<4 x i32>",
        "ANY",
        "<5 x i32>",
        "<16 x i8>*",
        "float",
        "<4 x i8*>",
        "void",
        "i8",
        "i16",
        "<16 x i32>",
        "<16 x i1>",
        "<5 x float>",
        "<2 x float>",
        "<16 x i8>",
        "i8*",
        "i32",
        "<1 x i8>",
        "float (<2 x float>, i32)",
        "<2 x float> (float, float)",
        "<5 x float> (<2 x float>, <2 x float>)",
        "i32 (i8*, i8)",
        "void (<4 x i8*>, <4 x i32>)"
        )
    )
  }


  "AST" in {
    val insertv =  cpg.method.name("insert").ast.isLiteral.code("undef").astParent.isCall.head
    val shuffle = cpg.method.name("findbyte").ast.isCall.name("<operator>.shufflevector").l.head
    val extract = cpg.method.name("extract").ast.isCall.code("extractelement").l.head

    val gep1 = cpg.method.name("vectorGEP").ast.isIdentifier.name("A").astParent.isCall.argument.isCall.name("<operator>.pointerShift").head
    val gep2 = cpg.method.name("vectorGEP").ast.isIdentifier.name("B").astParent.isCall.argument.isCall.name("<operator>.pointerShift").head
    val gep3 = cpg.method.name("vectorGEP").ast.isIdentifier.name("C").astParent.isCall.argument.isCall.name("<operator>.pointerShift").head

    //Todo: use treedump-like?
    argSummary(insertv) shouldBe Set((1,"undef","<2 x float>"), (2,"x","float"), (3,"1","i8"))
    argSummary(shuffle) shouldBe Set[Any]((1, "tmp", "<1 x i8>"), (2, "undef", "<1 x i8>"), (3, "zero initialized", "<16 x i32>"))
    argSummary(extract) shouldBe Set[Any]((1, "x", "<2 x float>"), (2, "idx", "i32"))

    argSummary(gep1) shouldBe Set[Any]((1, "ptrs", "<4 x i8*>"), (2, "offsets", "<4 x i32>"))
    argSummary(gep2) shouldBe Set[Any]((1, "ptrs", "<4 x i8*>"), (2, "5", "i32"))
    argSummary(gep3) shouldBe Set[Any]((1, "P1", "i8*"), (2, "offsets", "<4 x i32>"))
  }

  "CFG" in {
    val assignCallExtract = cpg.method.name("extract").block.astChildren.isCall.l.head
    val resTarget = assignCallExtract.start.astChildren.isIdentifier.head
    val extraction = assignCallExtract.start.astChildren.isCall.head
    val x = extraction.start.astChildren.isIdentifier.l.head
    val idx = extraction.start.astChildren.isIdentifier.l.last

    cpg.method.name("extract").cfgFirst.l shouldBe List(resTarget)
    resTarget.start.cfgNext.l shouldBe List(x)
    x.start.cfgNext.l shouldBe List(idx)
    idx.start.cfgNext.l shouldBe List(extraction)
    extraction.start.cfgNext.l shouldBe List(assignCallExtract)

  }
}
