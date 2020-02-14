package io.shiftleft.llvm2cpgintegration

import io.shiftleft.SerializedCpg
import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._
import io.shiftleft.semanticcpg.layers.EnhancementRunner
import org.scalatest.BeforeAndAfterAll

class StructMerging_01_Test extends CPGMatcher with BeforeAndAfterAll {
  private val cpg = CpgLoader.load(TestCpgPaths.StructMerging_01_TestCPG)
  override def beforeAll(): Unit = {
    val enhancement = new EnhancementRunner()
    enhancement.run(cpg, new SerializedCpg())
  }

  "merge structs" in {
    validateTypes(cpg, Set("ANY", "i32", "void",
      "Same", "Point", "TuplePair", "Point_1", "Point_2",
      "void (Same)", "void (Point)", "void (TuplePair)", "void (Point_1)", "void (Point_2)"))
  }
}
