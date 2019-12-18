package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._
import org.scalatest.{Matchers, WordSpec}
import io.shiftleft.codepropertygraph.generated.nodes._


class C_DbgVectorfindTest extends CPGMatcher {
  private val cpg = CpgLoader.load(TestCpgPaths.C_DbgVectorfindTestCPG)
  "Parameter Names" in {
    val params = cpg.method.name("findbyte").parameter.l
    params.map{p=>(p.name, p.order)}.toSet[Any] shouldBe Set(("haystack.arg", 1), ("needle.arg", 2), ("bogus.arg", 3))
    cpg.method.name("fooSingleton").parameter.l shouldBe List()
    cpg.method.name("extractX").parameter.name.l shouldBe List("point.arg", "arg1")
  }

  "Local Names" in {
    cpg.method.name("findbyte").block.local.nameNot("^(tmp|local).*").name.l.toSet shouldBe Set("cmpres.addr", "bogus.addr", "haystackv.addr", "haystack.addr", "needle.addr")
  }

  "Line numbers" in {
    cpg.method.name("findbyte").lineNumber.l shouldBe List(5)
    cpg.method.name("findbyte").ast.isCall.lineNumber(9).l.map{_.columnNumber}.toSet shouldBe Set(Some(13), Some(22), Some(37), Some(48), Some(62))
    cpg.method.name("findbyte").ast.isCall.lineNumber(9).l.map{_.columnNumber}.toSet.toList.sorted shouldBe List(
      Some(13),
      Some(22),
      Some(37),
      Some(48),
      Some(62)
    )
    cpg.method.name("findbyte").ast.isCall.lineNumber(9).l.filter(_.columnNumber == Some(13)).map{_.name}.toSet shouldBe Set[String](
      "<operator>.assignment",
      "<operator>.indirection"
    )
    cpg.method.name("findbyte").ast.isCall.lineNumber(9).l.filter(_.columnNumber == Some(22)).map{_.name}.toSet shouldBe Set[String](
    "<operator>.assignment",
    "<operator>.indirection",
    "<operator>.cast",
    "<operator>.equals"
    )
    cpg.method.name("findbyte").ast.isCall.lineNumber(9).l.filter(_.columnNumber == Some(37)).map{_.name}.toSet shouldBe Set[String](
      "<operator>.assignment",
      "<operator>.indirection"
    )
    cpg.method.name("findbyte").ast.isCall.lineNumber(9).l.filter(_.columnNumber == Some(48)).map{_.name}.toSet shouldBe Set(
      "<operator>.assignment",
      "<operator>.indirection",
      "<operator>.insertElement",
      "<operator>.cast"
    )
    cpg.method.name("findbyte").ast.isCall.lineNumber(9).l.filter(_.columnNumber == Some(62)).map{_.name}.toSet shouldBe Set(
      "<operator>.assignment",
      "<operator>.indirection"
    )
  }
}