package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.Cpg
import org.scalatest.{AppendedClues, Matchers, WordSpec}
import io.shiftleft.semanticcpg.language._

abstract class CPGMatcher extends WordSpec with Matchers with AppendedClues {
  def validateTypes(cpg: Cpg, types: Set[String]) {
    cpg.typeDecl.name.toSet shouldBe types
    cpg.types.name.toSet shouldBe types
  }
}
