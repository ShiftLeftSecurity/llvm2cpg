package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._
import overflowdb.traversal.NodeOps

class C_ReturnConstantTest extends CPGMatcher {
  private val cpg = CpgLoader.load(TestCpgPaths.C_ReturnConstantTestCPG)
  private val methodName = "basic_c_support"

  "metadata" in {
    cpg.metaData.l.size shouldBe 1
    cpg.metaData.head.language shouldBe "LLVM"
    cpg.metaData.head.version shouldBe "0"
  }

  "files" in {
    cpg.file.toList.size shouldBe 1
  }

  "types" in {
    validateTypes(cpg, List("ANY", "i32", "i32 ()"))
  }

  "methods" in {
    cpg.method.name(methodName).l.size shouldBe 1
  }

  "method return" in {
    cpg.method.name(methodName).l.size shouldBe 1
    val method = cpg.method.name(methodName).head
    method.start.methodReturn.l.size shouldBe 1
    val methodReturn = method.start.methodReturn.head
    methodReturn.typeFullName shouldBe "i32"
    methodReturn.code shouldBe "i32"
  }

  "method parameters" in {
    cpg.method.name(methodName).l.size shouldBe 1
    val method = cpg.method.name(methodName).head
    method.start.parameter.l.size shouldBe 0
  }

  "method block" in {
    val method = cpg.method.name(methodName).head
    method.start.block.l.size shouldBe 1
  }

  "method AST" in {
    val method = cpg.method.name(methodName).head
    val block = method.start.block.head
    block.start.astChildren.l.size shouldBe 1

    val ret = block.start.astChildren.isReturn.head
    ret.code shouldBe "return"

    ret.start.astChildren.l.size shouldBe 1
    val child = ret.start.astChildren.isLiteral.head
    child.code shouldBe "42"
    child.typeFullName shouldBe "i32"
    child.order shouldBe 1
    child.start.cfgNext.head shouldBe ret
  }

  "CFG" in {
    val method = cpg.method.name(methodName).head
    val block = method.start.block.head

    val ret = block.start.astChildren.isReturn.head
    val retValue = ret.start.astChildren.isLiteral.head
    retValue.start.cfgNext.head shouldBe ret
    method.start.cfgFirst.head shouldBe retValue
    val methodReturn = method.start.methodReturn.head
    methodReturn.start.cfgLast.head shouldBe ret
  }

}
