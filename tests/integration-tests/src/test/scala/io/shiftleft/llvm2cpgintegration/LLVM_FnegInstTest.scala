package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._
import org.scalatest.{Matchers, WordSpec}

class LLVM_FnegInstTest extends WordSpec with Matchers {
  private val cpg = CpgLoader.load(TestCpgPaths.LLVM_FnegInstCPG)
  private val methodName = "negate"

  "AST" in {
  /*
    %n = fneg float %x
    ret float %n
  */

    val method = cpg.method.name(methodName).head
    val block = method.start.block.head

    val x = method.start.parameter.head
    x.name shouldBe "x"

    val n = block.start.local.head
    n.name shouldBe "n"

    block.start.astChildren.isReturnNode.l.size shouldBe 1
    block.start.astChildren.isCall.l.size shouldBe 1

    val assignFneg = block.start.astChildren.isCall.head
    val selRef = assignFneg.start.astChildren.isIdentifier.head
    selRef.name shouldBe "n"
    selRef.start.refsTo.head shouldBe n

    val fnegCall = assignFneg.start.astChildren.isCall.head
    fnegCall.name shouldBe "fneg"

    val fnegArgument = fnegCall.start.astChildren.isIdentifier.l.head
    fnegArgument.code shouldBe "x"
    fnegArgument.typeFullName shouldBe "float"
    fnegArgument.start.refsTo.head shouldBe x

    val ret = block.start.astChildren.isReturnNode.head
    val retRef = ret.start.astChildren.isIdentifier.head
    retRef.name shouldBe "n"
    retRef.start.refsTo.head shouldBe n
  }

  "CFG" in {
  /*
    %n = fneg float %x
    ret float %n
  */
    val method = cpg.method.name(methodName).head
    val block = method.start.block.head

    val assignFneg = block.start.astChildren.isCall.head
    val fnegRef = assignFneg.start.astChildren.isIdentifier.head
    val fnegCall = assignFneg.start.astChildren.isCall.head
    val fnegArgument = fnegCall.start.astChildren.isIdentifier.l.head
    val ret = block.start.astChildren.isReturnNode.head
    val retRef = ret.start.astChildren.isIdentifier.head

    method.start.cfgFirst.head shouldBe fnegRef
    fnegRef.start.cfgNext.head shouldBe fnegArgument
    fnegArgument.start.cfgNext.head shouldBe fnegCall
    fnegCall.start.cfgNext.head shouldBe assignFneg
    assignFneg.start.cfgNext.head shouldBe retRef
    retRef.start.cfgNext.head shouldBe ret
    method.start.methodReturn.cfgLast.head shouldBe ret
  }

}