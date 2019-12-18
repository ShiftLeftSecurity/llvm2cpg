package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._
import org.scalatest.{Matchers, WordSpec}
import io.shiftleft.codepropertygraph.generated.nodes._


class LLVM_ConstantsTest extends CPGMatcher {
  private val cpg = CpgLoader.load(TestCpgPaths.LLVM_ConstantsTestCPG)
  "constants" in {
    cpg.method.ast.isLiteral.code.l.toSet shouldBe Set(
      "[4 x i16] [i16 1, i16 2, i16 3, i16 4]",
      "{ i32, i32, i32 } { i32 1, i32 2, i32 5 }",
      "abc",
      "<4 x i8> <i8 1, i8 1, i8 2, i8 3>",
      "<4 x i8> <i8 1, i8 undef, i8 2, i8 3>",
      "[4 x i8] c\"+*\\00n\"",
      "[4 x i8] c\"A\\00B\\00\"",
      "[4 x i8*] [i8* undef, i8* undef, i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str, i64 0, i64 0), i8* undef]",
      "[4 x i8] c\"abcd\""
    )
  }
}