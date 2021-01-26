package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._
import io.shiftleft.codepropertygraph.generated.nodes._

class C_APinliningTest extends CPGMatcher {
  private val cpg = CpgLoader.load(TestCpgPaths.C_APinliningTestCPG)
  val aliasing = cpg.method.name("nest_alias").head
  val noaliasing = cpg.method.name("nest_noalias").head
  "ast" in{
    // an assignment and a return node
    cpg.method.name("nest_noalias").block.astChildren.l.size shouldBe 2

    val alias_topnodes = cpg.method.name("nest_alias").block.astChildren.toList
    alias_topnodes.size shouldBe 4
    alias_topnodes.filter{_.isInstanceOf[Local]}.size shouldBe 1
    alias_topnodes.isReturn.l.size shouldBe 1
    alias_topnodes.isCall.name("<operator>.assignment").l.size shouldBe 2
    //nothing is inlined into the return Node
    alias_topnodes.isReturn.astChildren.isIdentifier.l.size shouldBe 1

  }
}
