package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._

/*
  %pointer.addr = alloca void (...)*, align 8
  store void (...)* %pointer, void (...)** %pointer.addr, align 8
  %0 = load void (...)*, void (...)** %pointer.addr, align 8
  call void (...) %0()
  ret void
 */
class C_CallFunctionPointerTest extends CPGMatcher {
  private val cpg = CpgLoader.load(TestCpgPaths.C_CallFunctionPointerCPG)
  private val methodName = "basic_c_support"

  "types" in {
    validateTypes(cpg, Set("ANY", "void (...)*", "void (...)**", "void", "void (void (...)*)"))
  }

  "AST" in {
    val method = cpg.method.name(methodName).head
    val block = method.start.block.head

    block.start.local.l.size shouldBe 2

    val tmpValue = block.start.local.l.apply(1)
    tmpValue.name shouldBe "tmp"
    tmpValue.typeFullName shouldBe "void (...)*"

    val ptrCall = block.start.astChildren.isCall.l.apply(2)
    ptrCall.name shouldBe "fptr"
    ptrCall.methodFullName shouldBe "fptr"
    ptrCall.typeFullName shouldBe "void"

    val receiver = ptrCall.start.receiver.isIdentifier.head
    receiver.name shouldBe "tmp"
    receiver.start.refsTo.head shouldBe tmpValue
    receiver.typeFullName shouldBe "void (...)*"
  }

  "CPG" in {
    val method = cpg.method.name(methodName).head
    val block = method.start.block.head

    val assignLoadCall = block.start.astChildren.isCall.l.apply(1)
    val ptrCall = block.start.astChildren.isCall.l.apply(2)
    val receiver = ptrCall.start.receiver.isIdentifier.head
    val ret = block.start.astChildren.isReturnNode.head

    assignLoadCall.start.cfgNext.head shouldBe receiver
    receiver.start.cfgNext.head shouldBe ptrCall
    ptrCall.start.cfgNext.head shouldBe ret
  }

}
