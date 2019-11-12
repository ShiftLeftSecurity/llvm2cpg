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
/* Starting from Method, generates lists of all AST descendants with nontrivial CFG (first non-RETURN nodes with !=1 predecessors, then non-entry nodes with != 1 successors)*/
  def getNonStraightCfg(meth: Method) = {
    val instructions = meth.start.block.ast.l.filter{n => n.isInstanceOf[Expression] && !n.isInstanceOf[Return] && !n.isInstanceOf[Block]}.asInstanceOf[List[Expression]]
    val first = meth.start.cfgFirst.head
    List(
     instructions.filter{ins => ins.start.cfgNext.l.size != 1},
     instructions.filter{ins => ins.start.cfgPrev.l.size != 1 && ins != first}
    )
  }
  /*starting from a call, return a summary of its arguments*/
  def argSummary(call: Call) = {
    call.start.argument.l.map{n=> (n.asInstanceOf[HasArgumentIndex].argumentIndex, n.asInstanceOf[HasCode].code, n.asInstanceOf[HasTypeFullName].typeFullName)}.toSet[Any]
  }

  def treeDump(root: Any):Any = {
    if(root.isInstanceOf[Literal]){
      val r_ = root.asInstanceOf[Literal]
      return ("LITERAL", r_.code, r_.typeFullName)
    }
    if(root.isInstanceOf[Identifier]){
      val r_ = root.asInstanceOf[Identifier]
      return ("IDENTIFIER", r_.code, r_.typeFullName)
    }
    if(root.isInstanceOf[Call]){
      val r_ = root.asInstanceOf[Call]
      val args = r_.start.argument.l.map{arg => (arg.asInstanceOf[HasArgumentIndex].argumentIndex, treeDump(arg))}.sortBy{_._1}

      return ("CALL", r_.name, r_.methodFullName, r_.code, r_.typeFullName, args)
    }
    if(root.isInstanceOf[Block]){
      val r_ = root.asInstanceOf[Block]
      val args = r_.start.astChildren.l.map{arg => (arg.order, treeDump(arg))}.sortBy{_._1}
      return ("BLOCK", r_.code, args)
    }
  return ("UNKNOWN", root.getClass)
  }
}
