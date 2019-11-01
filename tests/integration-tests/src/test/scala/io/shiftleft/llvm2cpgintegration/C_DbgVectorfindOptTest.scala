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
  }

  "Local Names" in {
    cpg.method.name("findbyte").block.local.nameNot("^(tmp|local).*").name.l.toSet should (
      equal (Set("bogus")) 
      or equal (Set("bogus", "haystackv.derived"))
    )
  }
}