package io.shiftleft.llvm2cpgintegration

import io.shiftleft.SerializedCpg
import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._
import io.shiftleft.semanticcpg.layers.EnhancementRunner
import org.scalatest.BeforeAndAfterAll

class ObjC_StringsTest extends CPGMatcher with BeforeAndAfterAll {
  private val cpg = CpgLoader.load(TestCpgPaths.ObjC_StringsTestCPG)
  override def beforeAll(): Unit = {
    val enhancement = new EnhancementRunner()
    enhancement.run(cpg, new SerializedCpg())
  }

  "strings" in {
    cpg.method("use").block.astChildren.isCall.order(1).isCall.argument.isLiteral.code.head shouldBe "Hello"
    cpg.method("use").block.astChildren.isCall.order(2).isCall.argument.isLiteral.code.head shouldBe "Hello"
    cpg.method("use").block.astChildren.isCall.order(3).isCall.argument.isLiteral.code.head shouldBe "world"
    cpg.method("use").block.astChildren.order(4).isCall.argument.isCall.argument.isCall.argument.isLiteral.code.head shouldBe "world"
  }

  "selectors" in {
    cpg.method("send").callOut.name("createObject").argument.isLiteral.code.head shouldBe "createObject"
  }

}
