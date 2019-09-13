package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import org.scalatest.{Matchers, WordSpec}
import io.shiftleft.semanticcpg.language._

class HelloWorldTest extends WordSpec with Matchers {
  val cpg = CpgLoader.load(TestCpgPaths.helloWorldCPG)

  "files" in {
    cpg.file.toList.size shouldBe 2
  }
  "methods" in {
    cpg.file.toList.size shouldBe 2
  }
}
