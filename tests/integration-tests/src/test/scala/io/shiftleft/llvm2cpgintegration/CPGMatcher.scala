package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.Cpg
import org.scalatest.{AppendedClues, Matchers, WordSpec}
import io.shiftleft.semanticcpg.language._
import io.shiftleft.codepropertygraph.generated.nodes._
import io.shiftleft.semanticcpg.language.types.expressions.generalizations

abstract class CPGMatcher extends WordSpec with Matchers with AppendedClues {
  def validateTypes(cpg: Cpg, types: Set[String]) {
    cpg.typeDecl.name.toSet shouldBe types
    cpg.types.name.toSet shouldBe types
  }

  /*starting from a call, return a summary of its arguments*/
  def argSummary(call: Call) = {
    call.start.argument.l.map{n=> (n.asInstanceOf[HasArgumentIndex].argumentIndex, n.asInstanceOf[HasCode].code, n.asInstanceOf[HasTypeFullName].typeFullName)}.toSet[Any]
  }
}
