package io.shiftleft.llvm2cpgintegration

import io.shiftleft.SerializedCpg
import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._
import io.shiftleft.semanticcpg.layers.EnhancementRunner
import org.scalatest.BeforeAndAfterAll

class ObjC_StringsNoInlineTest extends CPGMatcher with BeforeAndAfterAll {
  private val cpg = CpgLoader.load(TestCpgPaths.ObjC_StringsNoInlineTestCPG)
  override def beforeAll(): Unit = {
    val enhancement = new EnhancementRunner()
    enhancement.run(cpg, new SerializedCpg())
  }

  "strings" in {
    cpg.method("use").block.astChildren.order(1).isCall.argument.isCall.argument.isCall.argument.isCall.argument.code.head shouldBe "Hello"
  }

  "selectors" in {
    cpg.method("send").callOut.name("createObject").argument.isCall.argument.isIdentifier.code.head shouldBe "OBJC_SELECTOR_REFERENCES_"
  }

}
