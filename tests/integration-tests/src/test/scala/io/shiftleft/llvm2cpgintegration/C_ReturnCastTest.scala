package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._

class C_ReturnCastTest extends CPGMatcher {
  private val cpg = CpgLoader.load(TestCpgPaths.C_ReturnCastCPG)
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
    validateTypes(cpg, Set("ANY", "i8", "i32", "i8*"))
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
    param.typeFullName shouldBe "i8"
  }

  "method block" in {
    val method = cpg.method.name(methodName).head
    method.start.block.l.size shouldBe 1
  }

  "method locals" in {
    val method = cpg.method.name(methodName).head
    val locals = method.start.block.head.start.local.l
    locals.size shouldBe 3

    val xaddr = locals.head
    xaddr.name shouldBe "x.addr"
    xaddr.typeFullName shouldBe "i8*"
    xaddr.order shouldBe 0

    val tmp = locals.apply(1)
    tmp.name shouldBe "tmp"
    tmp.typeFullName shouldBe "i8"
    tmp.order shouldBe 1

    val conv = locals.last
    conv.name shouldBe "conv"
    conv.typeFullName shouldBe "i32"
    conv.order shouldBe 2
  }

  "method AST" in {
    /*
      %x.addr = alloca i8
      store i8 %x, i8* %x.addr
      %tmp = load i8, i8* %x.addr
      %conv = sext i8 %tmp to i32
      ret i32 %conv
    */

    val method = cpg.method.name(methodName).head
    val block = method.start.block.head
    block.start.astChildren.l.size shouldBe 8
    block.start.astChildren.isCall.l.size shouldBe 4
    block.start.astChildren.isReturnNode.l.size shouldBe 1

    val param = method.start.parameter.head
    val locals = block.start.local.l
    val xaddr = locals.head
    val tmp = locals.apply(1)
    val conv = locals.last

    {
      // %x.addr = alloca i8
      val assignCall = block.start.astChildren.isCall.l.head
      assignCall.name shouldBe "<operator>.assignment"
      assignCall.methodFullName shouldBe "<operator>.assignment"
      assignCall.order shouldBe 1
      assignCall.argumentIndex shouldBe 1
      assignCall.typeFullName shouldBe "i8*"

      assignCall.start.astChildren.isIdentifier.l.size shouldBe 1
      val lhs = assignCall.start.astChildren.isIdentifier.head
      lhs.name shouldBe "x.addr"
      lhs.start.refsTo.l.size shouldBe 1
      lhs.start.refsTo.head shouldBe xaddr
      lhs.order shouldBe 1
      lhs.argumentIndex shouldBe 1

      assignCall.start.astChildren.isCall.l.size shouldBe 1
      val rhs = assignCall.start.astChildren.isCall.head
      rhs.name shouldBe "<operator>.alloca"
      rhs.typeFullName shouldBe "i8*"
      rhs.order shouldBe 2
      rhs.argumentIndex shouldBe 2
    }

    {
      // store i8 %x, i8* %x.addr
      val assignCall = block.start.astChildren.isCall.l.apply(1)
      assignCall.name shouldBe "<operator>.assignment"
      assignCall.order shouldBe 2
      assignCall.argumentIndex shouldBe 2
      assignCall.start.astChildren.l.size shouldBe 2

      assignCall.start.astChildren.isCall.l.size shouldBe 1
      val lhs = assignCall.start.astChildren.isCall.l.head
      lhs.name shouldBe "<operator>.indirection"
      lhs.methodFullName shouldBe "<operator>.indirection"
      lhs.typeFullName shouldBe "i8"
      lhs.order shouldBe 1
      lhs.argumentIndex shouldBe 1

      lhs.start.astChildren.l.size shouldBe 1
      lhs.start.astChildren.isIdentifier.l.size shouldBe 1
      val indirection = lhs.start.astChildren.isIdentifier.l.head
      indirection.name shouldBe "x.addr"
      indirection.typeFullName shouldBe "i8*"
      indirection.start.refsTo.head shouldBe xaddr
      indirection.order shouldBe 1
      indirection.argumentIndex shouldBe 1

      assignCall.start.astChildren.isIdentifier.l.size shouldBe 1
      val rhs = assignCall.start.astChildren.isIdentifier.l.head
      rhs.name shouldBe "x"
      rhs.typeFullName shouldBe "i8"
      rhs.order shouldBe 2
      rhs.argumentIndex shouldBe 2
      rhs.start.refsTo.l.size shouldBe 1
      rhs.start.refsTo.head shouldBe param
    }

    {
      // %tmp = load i8, i8* %x.addr
      val assignCall = block.start.astChildren.isCall.l.apply(2)
      assignCall.name shouldBe "<operator>.assignment"
      assignCall.order shouldBe 3
      assignCall.argumentIndex shouldBe 3
      assignCall.start.astChildren.l.size shouldBe 2

      assignCall.start.astChildren.isIdentifier.l.size shouldBe 1
      val lhs =  assignCall.start.astChildren.isIdentifier.head
      lhs.name shouldBe "tmp"
      lhs.typeFullName shouldBe "i8"
      lhs.start.refsTo.l.size shouldBe 1
      lhs.start.refsTo.head shouldBe tmp
      lhs.order shouldBe 1
      lhs.argumentIndex shouldBe 1

      assignCall.start.astChildren.isCall.l.size shouldBe 1
      val rhs =  assignCall.start.astChildren.isCall.head
      rhs.name shouldBe "<operator>.indirection"
      rhs.methodFullName shouldBe "<operator>.indirection"
      rhs.typeFullName shouldBe "i8*"
      rhs.order shouldBe 2
      rhs.argumentIndex shouldBe 2
      rhs.start.astChildren.l.size shouldBe 1
      rhs.start.astChildren.isIdentifier.l.size shouldBe 1
      val dereference = rhs.start.astChildren.isIdentifier.head
      dereference.name shouldBe "x.addr"
      dereference.typeFullName shouldBe "i8*"
      dereference.order shouldBe 1
      dereference.argumentIndex shouldBe 1
      dereference.start.refsTo.l.size shouldBe 1
      dereference.start.refsTo.head shouldBe xaddr
    }

    {
      // %conv = sext i8 %tmp to i32
      val assignCall = block.start.astChildren.isCall.l.apply(3)
      assignCall.name shouldBe "<operator>.assignment"
      assignCall.order shouldBe 4
      assignCall.argumentIndex shouldBe 4
      assignCall.start.astChildren.l.size shouldBe 2

      assignCall.start.astChildren.isIdentifier.l.size shouldBe 1
      val lhs =  assignCall.start.astChildren.isIdentifier.head
      lhs.name shouldBe "conv"
      lhs.typeFullName shouldBe "i32"
      lhs.start.refsTo.l.size shouldBe 1
      lhs.start.refsTo.head shouldBe conv
      lhs.order shouldBe 1
      lhs.argumentIndex shouldBe 1

      assignCall.start.astChildren.isCall.l.size shouldBe 1
      val rhs =  assignCall.start.astChildren.isCall.head
      rhs.name shouldBe "<operator>.cast"
      rhs.methodFullName shouldBe "<operator>.cast"
      rhs.typeFullName shouldBe "i32"
      rhs.order shouldBe 2
      rhs.argumentIndex shouldBe 2
      rhs.start.astChildren.l.size shouldBe 1
      rhs.start.astChildren.isIdentifier.l.size shouldBe 1
      val cast = rhs.start.astChildren.isIdentifier.head
      cast.name shouldBe "tmp"
      cast.typeFullName shouldBe "i8"
      cast.order shouldBe 1
      cast.argumentIndex shouldBe 1
      cast.start.refsTo.l.size shouldBe 1
      cast.start.refsTo.head shouldBe tmp
    }

    {
      // ret i32 %conv
      val ret = block.start.astChildren.isReturnNode.l.last
      ret.code shouldBe "return"
      ret.order shouldBe 5
      ret.argumentIndex shouldBe 5

      ret.start.astChildren.l.size shouldBe 1
      ret.start.astChildren.isIdentifier.l.size shouldBe 1
      val retVal = ret.start.astChildren.isIdentifier.head

      retVal.name shouldBe "conv"
      retVal.order shouldBe 1
      retVal.argumentIndex shouldBe 1
      retVal.start.refsTo.l.size shouldBe 1
      retVal.start.refsTo.head shouldBe conv
    }

  }

  "CFG" in {
    /*
      %x.addr = alloca i8
      store i8 %x, i8* %x.addr
      %tmp = load i8, i8* %x.addr
      %conv = sext i8 %tmp to i32
      ret i32 %conv
    */

    val method = cpg.method.name(methodName).head
    val block = method.start.block.head

    val assignAllocaCall = block.start.astChildren.isCall.l.head
    val assignStoreCall = block.start.astChildren.isCall.l.apply(1)
    val assignLoadCall = block.start.astChildren.isCall.l.apply(2)
    val assignCastCall = block.start.astChildren.isCall.l.apply(3)

    {
      // %x.addr = alloca i32
      val lhs = assignAllocaCall.start.astChildren.isIdentifier.head
      val rhs = assignAllocaCall.start.astChildren.isCall.head

      lhs.start.cfgNext.head shouldBe rhs
      rhs.start.cfgNext.head shouldBe assignAllocaCall

      method.start.cfgFirst.head shouldBe lhs
    }

    {
      // store i32 %x, i32* %x.addr
      val lhs = assignStoreCall.start.astChildren.isCall.l.head
      val ref = lhs.start.astChildren.isIdentifier.l.head
      val rhs = assignStoreCall.start.astChildren.isIdentifier.l.head

      ref.start.cfgNext.head shouldBe lhs
      lhs.start.cfgNext.head shouldBe rhs
      rhs.start.cfgNext.head shouldBe assignStoreCall

      assignAllocaCall.start.cfgNext.head shouldBe ref
    }

    {
      // %tmp = load i32, i32* %x.addr
      val lhs =  assignLoadCall.start.astChildren.isIdentifier.head
      val rhs =  assignLoadCall.start.astChildren.isCall.head
      val ref = rhs.start.astChildren.isIdentifier.head

      lhs.start.cfgNext.head shouldBe ref
      ref.start.cfgNext.head shouldBe rhs
      rhs.start.cfgNext.head shouldBe assignLoadCall
      assignStoreCall.start.cfgNext.head shouldBe lhs
    }

    {
      // %conv = sext i8 %tmp to i32
      val lhs =  assignCastCall.start.astChildren.isIdentifier.head
      val rhs =  assignCastCall.start.astChildren.isCall.head
      val ref = rhs.start.astChildren.isIdentifier.head

      lhs.start.cfgNext.head shouldBe ref
      ref.start.cfgNext.head shouldBe rhs
      rhs.start.cfgNext.head shouldBe assignCastCall
      assignLoadCall.start.cfgNext.head shouldBe lhs
    }

    {
      // ret i32 %conv
      val ret = block.start.astChildren.isReturnNode.l.last
      val retVal = ret.start.astChildren.isIdentifier.head
      retVal.start.cfgNext.head shouldBe ret
      assignCastCall.start.cfgNext.head shouldBe retVal

      val methodReturn = method.start.methodReturn.head
      methodReturn.start.cfgLast.head shouldBe ret
    }
  }

}
