package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.Cpg
import org.scalatest.{AppendedClues, Matchers, WordSpec}
import io.shiftleft.semanticcpg.language._
import io.shiftleft.codepropertygraph.generated.nodes._
import io.shiftleft.semanticcpg.language.types.expressions.generalizations

abstract class CPGMatcher extends WordSpec with Matchers with AppendedClues {
  def validateTypes(cpg: Cpg, types: List[String]) {
    val expectation = types.prepended("i32").prepended("i64").toSet.toList.sorted
    val cpgDecls = cpg.typeDecl.name.toList.prepended("i32").prepended("i64").toSet.toList.sorted
    val cpgTypes = cpg.types.name.toList.prepended("i32").prepended("i64").toSet.toList.sorted
    cpgDecls shouldBe expectation
    cpgTypes shouldBe expectation
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

  def treeDump(root: Any, stringify: scala.collection.mutable.StringBuilder = null, depth:Int = 0):Any = {
    if(root.isInstanceOf[Literal]){
      val r_ = root.asInstanceOf[Literal]
      if(stringify != null) {
        stringify.append("\t"*depth).append(s"LITERAL code = ${r_.code} type = ${r_.typeFullName}\n")
        return stringify

      }
      return ("LITERAL", r_.code, r_.typeFullName)
    }
    if(root.isInstanceOf[Identifier]){
      val r_ = root.asInstanceOf[Identifier]
      if(stringify != null) {
        stringify.append("\t"*depth).append(s"IDENTIFIER code = ${r_.code} type = ${r_.typeFullName}\n")
        return stringify
      }
      return ("IDENTIFIER", r_.code, r_.typeFullName)
    }
    if(root.isInstanceOf[FieldIdentifier]){
      val r_ = root.asInstanceOf[FieldIdentifier]
      if(stringify != null) {
        stringify.append("\t"*depth).append(s"FIELD_IDENTIFIER code = ${r_.code} canonicalName = ${r_.canonicalName}\n")
        return stringify
      }
      return ("FIELD_IDENTIFIER", r_.code, r_.canonicalName)
    }
    if(root.isInstanceOf[Call]){
      val r_ = root.asInstanceOf[Call]
      if(stringify != null) {
        val args = r_.start.argument.l.map{arg => (arg.asInstanceOf[HasArgumentIndex].argumentIndex, treeDump(arg, new StringBuilder(), depth+1))}.sortBy{_._1}

        stringify.append("\t"*depth).append(s"CALL name = ${r_.name} fullname = ${r_.methodFullName} return type = ${r_.typeFullName}\n")
        args.map(a => stringify.append(a._2.asInstanceOf[StringBuilder].toString()))
        return stringify

      }
      val args = r_.start.argument.l.map{arg => (arg.asInstanceOf[HasArgumentIndex].argumentIndex, treeDump(arg))}.sortBy{_._1}
      return ("CALL", r_.name, r_.methodFullName, r_.code, r_.typeFullName, args)
    }
    if(root.isInstanceOf[Block]){
      val r_ = root.asInstanceOf[Block]
      if(stringify != null) {
        val args = r_.start.astChildren.l.map{arg => (arg.order, treeDump(arg,  new StringBuilder(), depth+1))}.sortBy{_._1}

        stringify.append("\t"*depth).append(s"(BLOCK nchildren = ${args.length})\n)")
        args.map(a => stringify.append(a._2.asInstanceOf[StringBuilder].toString()))
        return stringify

      }
      val args = r_.start.astChildren.l.map{arg => (arg.order, treeDump(arg))}.sortBy{_._1}
      return ("BLOCK", r_.code, args)
    }
    if(root.isInstanceOf[Local]){
      val r_ = root.asInstanceOf[Local]
      if(stringify != null) {
        stringify.append("\t"*depth).append("LOCAL code = ${r_.code} name = ${r_.name}")
        return stringify
      }
      return ("LOCAL", r_.code, r_.name)
    }
    if(stringify != null) {
      stringify.append("\t"*depth).append(s"UNKNOWN class = ${root.getClass}")
      return stringify
    }
  return ("UNKNOWN", root.getClass)
  }
}
