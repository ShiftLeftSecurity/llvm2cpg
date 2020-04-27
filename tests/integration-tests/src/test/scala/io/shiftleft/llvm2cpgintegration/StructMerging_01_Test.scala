package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import org.scalatest.BeforeAndAfterAll

class StructMerging_01_Test extends CPGMatcher with BeforeAndAfterAll {
  private val cpg = CpgLoader.load(TestCpgPaths.StructMerging_01_TestCPG)

  "merge structs" in {
    validateTypes(cpg, List("ANY", "i32", "void",
      "Same", "Point", "TuplePair", "Point_1", "Point_2",
      "void (Same)", "void (Point)", "void (TuplePair)", "void (Point_1)", "void (Point_2)"))
  }
}
