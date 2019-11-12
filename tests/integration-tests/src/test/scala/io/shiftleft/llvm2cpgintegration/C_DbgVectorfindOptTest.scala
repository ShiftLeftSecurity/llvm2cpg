package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._
import org.scalatest.{Matchers, WordSpec}
import io.shiftleft.codepropertygraph.generated.nodes._


class C_DbgVectorfindOptTest extends CPGMatcher {
  private val cpg = CpgLoader.load(TestCpgPaths.C_DbgVectorfindOptCPG)
  "Parameter Names" in {
    val params = cpg.method.name("findbyte").parameter.l
    params.map{p=>(p.name, p.order)}.toSet[Any] shouldBe Set(("haystack.arg", 1), ("needle.arg", 2), ("bogus.arg", 3))
    cpg.method.name("fooSingleton").parameter.l shouldBe List()
    cpg.method.name("extractX").parameter.name.l shouldBe List("point.arg", "arg1")
  }

  "Local Names" in {
    cpg.method.name("findbyte").block.local.nameNot("^(tmp|local).*").name.l.toSet should (
      equal (Set("bogus")) 
      or equal (Set("bogus", "haystackv.derived"))
    )
  }

  "Line numbers" in {
    cpg.method.name("findbyte").lineNumber.l shouldBe List(5)
    cpg.method.name("findbyte").ast.isCall.lineNumber(9).l.map{_.columnNumber}.toSet shouldBe Set(Some(22), Some(48))

    cpg.method.name("findbyte").ast.isCall.lineNumber(9).l.filter(_.columnNumber == Some(22)).map{_.name}.toSet  shouldBe Set(
      "<operator>.assignment",
      "<operator>.equals"
    )

    cpg.method.name("findbyte").ast.isCall.lineNumber(9).l.filter(_.columnNumber == Some(48)).map{_.name}.toSet shouldBe Set(
      "<operator>.assignment",
      "<operator>.insertElement",
      "<operator>.shufflevector"
    )
  }
}