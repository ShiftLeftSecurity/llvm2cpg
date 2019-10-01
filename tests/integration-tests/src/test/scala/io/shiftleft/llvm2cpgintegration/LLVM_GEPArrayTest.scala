package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._
import org.scalatest.{Matchers, WordSpec}

/*
  %ptr = getelementptr inbounds i32, i32* %x, i64 1  ;  1
  ret i32* %ptr                                      ;  2
*/
class LLVM_GEPArrayTest extends WordSpec with Matchers {
  private val cpg = CpgLoader.load(TestCpgPaths.LLVM_GEPArrayCPG)
  private val methodName = "gep_array"

  "AST" in {
    val method = cpg.method.name(methodName).head
    val block = method.start.block.head

    val ptr = block.start.local.head
    val x = method.start.parameter.head

    // %ptr = getelementptr inbounds i32, i32* %x, i64 1
    val assignGEP = block.start.astChildren.isCall.head

    val xRef = assignGEP.start.astChildren.isIdentifier.head
    xRef.name shouldBe "ptr"
    xRef.start.refsTo.head shouldBe ptr

    val indexAccessGEP = assignGEP.start.astChildren.isCall.head
    indexAccessGEP.name shouldBe "index_access"
    indexAccessGEP.typeFullName shouldBe "i32*"

    val indexAccessGEP_index = indexAccessGEP.start.astChildren.isLiteral.head
    indexAccessGEP_index.code shouldBe "1"

    val xIdentifier = indexAccessGEP.start.astChildren.isIdentifier.head
    xIdentifier.name shouldBe "x"
    xIdentifier.start.refsTo.head shouldBe x
  }

  "CFG" in {
    val method = cpg.method.name(methodName).head
    val block = method.start.block.head

    // %ptr = getelementptr inbounds i32, i32* %x, i64 1
    val assignGEP = block.start.astChildren.isCall.head
    val ptrRef = assignGEP.start.astChildren.isIdentifier.head
    val indexAccessGEP = assignGEP.start.astChildren.isCall.head
    val indexAccessGEP_index = indexAccessGEP.start.astChildren.isLiteral.head
    val xIdentifier = indexAccessGEP.start.astChildren.isIdentifier.head

    ptrRef.start.cfgNext.head shouldBe xIdentifier
    xIdentifier.start.cfgNext.head shouldBe indexAccessGEP_index
    indexAccessGEP_index.start.cfgNext.head shouldBe indexAccessGEP
    indexAccessGEP.start.cfgNext.head shouldBe assignGEP

    // ret i32* %ptr
    val ret = block.start.astChildren.isReturnNode.head
    val retPtr = ret.start.astChildren.isIdentifier.head
    assignGEP.start.cfgNext.head shouldBe retPtr
  }

}
