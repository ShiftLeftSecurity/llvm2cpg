package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._

class C_ReturnParameterTest extends CPGMatcher {
  private val cpg = CpgLoader.load(TestCpgPaths.C_ReturnParameterCPG)
  private val methodName = "basic_c_support"

  "metadata" in {
    cpg.metaData.l.size shouldBe 1
    cpg.metaData.head.language shouldBe "C"
    cpg.metaData.head.version shouldBe "0"
  }

  "files" in {
    cpg.file.toList.size shouldBe 1
  }

  "types" in {
    validateTypes(cpg, Set("ANY", "i32", "i32*", "i32 (i32)"))
  }

  "methods" in {
    cpg.method.name(methodName).l.size shouldBe 1
  }

  "methodReturn" in {
    val method = cpg.method.name(methodName).head
    method.start.methodReturn.l.size shouldBe 1
    val methodReturn = method.start.methodReturn.head
    methodReturn.typeFullName shouldBe "i32"
    methodReturn.code shouldBe "i32"
  }

  "methodParameters" in {
    val method = cpg.method.name(methodName).head

    method.start.parameter.l.size shouldBe 1
    val param = method.start.parameter.head
    param.name shouldBe "x"
    param.order shouldBe 1
    param.typeFullName shouldBe "i32"
  }

  "method block" in {
    val method = cpg.method.name(methodName).head
    method.start.block.l.size shouldBe 1
  }

  "method locals" in {
    val method = cpg.method.name(methodName).head
    val locals = method.start.block.head.start.local.l
    locals.size shouldBe 2

    val xaddr = locals.head
    xaddr.name shouldBe "x.addr"
    xaddr.typeFullName shouldBe "i32*"
    xaddr.order shouldBe 0

    val tmp = locals.last
    tmp.name shouldBe "tmp"
    tmp.typeFullName shouldBe "i32"
    tmp.order shouldBe 1
  }

  "method AST" in {
    /*
      %x.addr = alloca i32
      store i32 %x, i32* %x.addr
      %0 = load i32, i32* %x.addr
      ret i32 %0
    */

    val method = cpg.method.name(methodName).head
    val block = method.start.block.head
    block.start.astChildren.l.size shouldBe 5
    block.start.astChildren.isCall.l.size shouldBe 2
    block.start.astChildren.isReturnNode.l.size shouldBe 1

    val param = method.start.parameter.head
    val locals = block.start.local.l
    val xaddr = locals.head
    val tmp = locals.last

    {
      // store i32 %x, i32* %x.addr
      val assignCall = block.start.astChildren.isCall.l.apply(0)
      assignCall.name shouldBe "<operator>.assignment"
      assignCall.order shouldBe 1
      assignCall.argumentIndex shouldBe 1
      assignCall.start.astChildren.l.size shouldBe 2

      assignCall.start.astChildren.isCall.l.size shouldBe 1
      val lhs = assignCall.start.astChildren.isCall.l.head
      lhs.name shouldBe "<operator>.indirection"
      lhs.typeFullName shouldBe "i32"
      lhs.order shouldBe 1
      lhs.argumentIndex shouldBe 1

      lhs.start.astChildren.l.size shouldBe 1
      lhs.start.astChildren.isIdentifier.l.size shouldBe 1
      val indirection = lhs.start.astChildren.isIdentifier.l.head
      indirection.name shouldBe "x.addr"
      indirection.typeFullName shouldBe "i32*"
      indirection.start.refsTo.head shouldBe xaddr
      indirection.order shouldBe 1
      indirection.argumentIndex shouldBe 1

      assignCall.start.astChildren.isIdentifier.l.size shouldBe 1
      val rhs = assignCall.start.astChildren.isIdentifier.l.head
      rhs.name shouldBe "x"
      rhs.typeFullName shouldBe "i32"
      rhs.order shouldBe 2
      rhs.argumentIndex shouldBe 2
      rhs.start.refsTo.l.size shouldBe 1
      rhs.start.refsTo.head shouldBe param
    }

    {
      // %0 = load i32, i32* %x.addr
      val assignCall = block.start.astChildren.isCall.l.apply(1)
      assignCall.name shouldBe "<operator>.assignment"
      assignCall.order shouldBe 2
      assignCall.argumentIndex shouldBe 2
      assignCall.start.astChildren.l.size shouldBe 2

      assignCall.start.astChildren.isIdentifier.l.size shouldBe 1
      val lhs =  assignCall.start.astChildren.isIdentifier.head
      lhs.name shouldBe "tmp"
      lhs.typeFullName shouldBe "i32"
      lhs.start.refsTo.l.size shouldBe 1
      lhs.start.refsTo.head shouldBe tmp
      lhs.order shouldBe 1
      lhs.argumentIndex shouldBe 1

      assignCall.start.astChildren.isCall.l.size shouldBe 1
      val rhs =  assignCall.start.astChildren.isCall.head
      rhs.name shouldBe "<operator>.indirection"
      rhs.typeFullName shouldBe "i32"
      rhs.order shouldBe 2
      rhs.argumentIndex shouldBe 2
      rhs.start.astChildren.l.size shouldBe 1
      rhs.start.astChildren.isIdentifier.l.size shouldBe 1
      val dereference = rhs.start.astChildren.isIdentifier.head
      dereference.name shouldBe "x.addr"
      dereference.typeFullName shouldBe "i32*"
      dereference.order shouldBe 1
      dereference.argumentIndex shouldBe 1
      dereference.start.refsTo.l.size shouldBe 1
      dereference.start.refsTo.head shouldBe xaddr
    }

    {
      // ret i32 %0
      val ret = block.start.astChildren.isReturnNode.l.last
      ret.code shouldBe "return"
      ret.order shouldBe 3
      ret.argumentIndex shouldBe 3

      ret.start.astChildren.l.size shouldBe 1
      ret.start.astChildren.isIdentifier.l.size shouldBe 1
      val retVal = ret.start.astChildren.isIdentifier.head

      retVal.name shouldBe "tmp"
      retVal.order shouldBe 1
      retVal.argumentIndex shouldBe 1
      retVal.start.refsTo.l.size shouldBe 1
      retVal.start.refsTo.head shouldBe tmp
    }

  }

  "CFG" in {
    /*
      %x.addr = alloca i32
      store i32 %x, i32* %x.addr
      %0 = load i32, i32* %x.addr
      ret i32 %0
    */

    val method = cpg.method.name(methodName).head
    val block = method.start.block.head

    val assignStoreCall = block.start.astChildren.isCall.l.apply(0)
    val assignLoadCall = block.start.astChildren.isCall.l.apply(1)


    {
      // store i32 %x, i32* %x.addr
      val lhs = assignStoreCall.start.astChildren.isCall.l.head
      val ref = lhs.start.astChildren.isIdentifier.l.head
      val rhs = assignStoreCall.start.astChildren.isIdentifier.l.head

      ref.start.cfgNext.head shouldBe lhs
      lhs.start.cfgNext.head shouldBe rhs
      rhs.start.cfgNext.head shouldBe assignStoreCall
    }

    {
      // %0 = load i32, i32* %x.addr
      val lhs =  assignLoadCall.start.astChildren.isIdentifier.head
      val rhs =  assignLoadCall.start.astChildren.isCall.head
      val ref = rhs.start.astChildren.isIdentifier.head

      lhs.start.cfgNext.head shouldBe ref
      ref.start.cfgNext.head shouldBe rhs
      rhs.start.cfgNext.head shouldBe assignLoadCall
      assignStoreCall.start.cfgNext.head shouldBe lhs
    }

    {
      // ret i32 %0
      val ret = block.start.astChildren.isReturnNode.l.last
      val retVal = ret.start.astChildren.isIdentifier.head
      retVal.start.cfgNext.head shouldBe ret
      assignLoadCall.start.cfgNext.head shouldBe retVal

      val methodReturn = method.start.methodReturn.head
      methodReturn.start.cfgLast.head shouldBe ret
    }
  }

}
