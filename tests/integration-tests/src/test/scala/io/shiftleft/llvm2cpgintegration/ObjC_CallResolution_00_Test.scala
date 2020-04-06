package io.shiftleft.llvm2cpgintegration

import io.shiftleft.SerializedCpg
import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._
import io.shiftleft.semanticcpg.layers.EnhancementRunner
import org.scalatest.BeforeAndAfterAll

class ObjC_CallResolution_00_Test extends CPGMatcher with BeforeAndAfterAll {
  private val cpg = CpgLoader.load(TestCpgPaths.ObjC_CallResolution_00_TestCPG)
  override def beforeAll(): Unit = {
    val enhancement = new EnhancementRunner()
    enhancement.run(cpg, new SerializedCpg())
  }

  /// Doesn't work with NoResolve
  "calls" ignore {
    val newMethod = cpg.method.fullNameExact("+[NSObject new]").head
    val description = cpg.method.fullNameExact("-[NSObject description]").head
    val resolver = NoResolve
    cpg.method.name("main").callOut.name("new").calledMethod(resolver).head shouldBe newMethod
    cpg.method.name("main").callOut.name("description").calledMethod(resolver).head shouldBe description
  }

}
