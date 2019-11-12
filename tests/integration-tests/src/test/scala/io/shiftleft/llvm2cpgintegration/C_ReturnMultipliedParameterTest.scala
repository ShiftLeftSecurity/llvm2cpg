package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._

class C_ReturnMultipliedParameterTest extends CPGMatcher {
  private val cpg = CpgLoader.load(TestCpgPaths.C_ReturnMultipliedParameterCPG)
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
    locals.size shouldBe 5

    val xaddr = locals.head
    xaddr.name shouldBe "x.addr"
    xaddr.typeFullName shouldBe "i32*"
    xaddr.order shouldBe 0

    val multiplier = locals.apply(1)
    multiplier.name shouldBe "m"
    multiplier.typeFullName shouldBe "i32*"
    multiplier.order shouldBe 1

    val tmp0 = locals.apply(2)
    tmp0.name shouldBe "tmp"
    tmp0.typeFullName shouldBe "i32"
    tmp0.order shouldBe 2

    val tmp1 = locals.apply(3)
    tmp1.name shouldBe "tmp1"
    tmp1.typeFullName shouldBe "i32"
    tmp1.order shouldBe 3

    val multiplicationResult = locals.apply(4)
    multiplicationResult.name shouldBe "mul"
    multiplicationResult.typeFullName shouldBe "i32"
    multiplicationResult.order shouldBe 4
  }

  /*
  %x.addr = alloca i32, align 4
  %m = alloca i32, align 4
  store i32 %x, i32* %x.addr, align 4
  store i32 42, i32* %m, align 4
  %0 = load i32, i32* %m, align 4
  %1 = load i32, i32* %x.addr, align 4
  %mul = mul nsw i32 %0, %1
  ret i32 %mul
  */

  "method AST" in {
    val method = cpg.method.name(methodName).head
    val block = method.start.block.head
    block.start.astChildren.l.size shouldBe 11
    block.start.astChildren.isCall.l.size shouldBe 5
    block.start.astChildren.isReturnNode.l.size shouldBe 1

    val param = method.start.parameter.head
    val locals = block.start.local.l
    val xaddr = locals.head
    val m = locals.apply(1)
    val tmp0 = locals.apply(2)
    val tmp1 = locals.apply(3)
    val mul = locals.apply(4)


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
      //  store i32 42, i32* %m, align 4
      val assignCall = block.start.astChildren.isCall.l.apply(1)
      assignCall.name shouldBe "<operator>.assignment"
      assignCall.order shouldBe 2
      assignCall.argumentIndex shouldBe 2
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
      indirection.name shouldBe "m"
      indirection.start.refsTo.head shouldBe m
      indirection.order shouldBe 1
      indirection.argumentIndex shouldBe 1

      assignCall.start.astChildren.isLiteral.l.size shouldBe 1
      val rhs = assignCall.start.astChildren.isLiteral.l.head
      rhs.code shouldBe "42"
      rhs.typeFullName shouldBe "i32"
      rhs.order shouldBe 2
      rhs.argumentIndex shouldBe 2
    }

    {
      // %0 = load i32, i32* %m
      val assignCall = block.start.astChildren.isCall.l.apply(2)
      assignCall.name shouldBe "<operator>.assignment"
      assignCall.order shouldBe 3
      assignCall.argumentIndex shouldBe 3
      assignCall.start.astChildren.l.size shouldBe 2

      assignCall.start.astChildren.isIdentifier.l.size shouldBe 1
      val lhs = assignCall.start.astChildren.isIdentifier.head
      lhs.name shouldBe "tmp"
      lhs.typeFullName shouldBe "i32"
      lhs.start.refsTo.l.size shouldBe 1
      lhs.start.refsTo.head shouldBe tmp0
      lhs.order shouldBe 1
      lhs.argumentIndex shouldBe 1

      assignCall.start.astChildren.isCall.l.size shouldBe 1
      val rhs = assignCall.start.astChildren.isCall.head
      rhs.name shouldBe "<operator>.indirection"
      rhs.typeFullName shouldBe "i32*"
      rhs.order shouldBe 2
      rhs.argumentIndex shouldBe 2
      rhs.start.astChildren.l.size shouldBe 1
      rhs.start.astChildren.isIdentifier.l.size shouldBe 1
      val dereference = rhs.start.astChildren.isIdentifier.head
      dereference.name shouldBe "m"
      dereference.typeFullName shouldBe "i32*"
      dereference.order shouldBe 1
      dereference.argumentIndex shouldBe 1
      dereference.start.refsTo.l.size shouldBe 1
      dereference.start.refsTo.head shouldBe m
    }

    {
      // %1 = load i32, i32* %x.addr
      val assignCall = block.start.astChildren.isCall.l.apply(3)
      assignCall.name shouldBe "<operator>.assignment"
      assignCall.order shouldBe 4
      assignCall.argumentIndex shouldBe 4
      assignCall.start.astChildren.l.size shouldBe 2

      assignCall.start.astChildren.isIdentifier.l.size shouldBe 1
      val lhs = assignCall.start.astChildren.isIdentifier.head
      lhs.name shouldBe "tmp1"
      lhs.typeFullName shouldBe "i32"
      lhs.start.refsTo.l.size shouldBe 1
      lhs.start.refsTo.head shouldBe tmp1
      lhs.order shouldBe 1
      lhs.argumentIndex shouldBe 1

      assignCall.start.astChildren.isCall.l.size shouldBe 1
      val rhs = assignCall.start.astChildren.isCall.head
      rhs.name shouldBe "<operator>.indirection"
      rhs.typeFullName shouldBe "i32*"
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
      //  %mul = mul nsw i32 %0, %1
      val assignCall = block.start.astChildren.isCall.l.apply(4)
      assignCall.name shouldBe "<operator>.assignment"
      assignCall.order shouldBe 5
      assignCall.argumentIndex shouldBe 5
      assignCall.start.astChildren.l.size shouldBe 2

      assignCall.start.astChildren.isIdentifier.l.size shouldBe 1
      val lhs = assignCall.start.astChildren.isIdentifier.head
      lhs.name shouldBe "mul"
      lhs.name shouldBe "mul"
      lhs.typeFullName shouldBe "i32"
      lhs.start.refsTo.l.size shouldBe 1
      lhs.start.refsTo.head shouldBe mul
      lhs.order shouldBe 1
      lhs.argumentIndex shouldBe 1

      assignCall.start.astChildren.isCall.l.size shouldBe 1
      val mulCall = assignCall.start.astChildren.isCall.head
      mulCall.name shouldBe "<operator>.multiplication"
      mulCall.typeFullName shouldBe "i32"
      mulCall.order shouldBe 2
      mulCall.argumentIndex shouldBe 2
      mulCall.start.astChildren.l.size shouldBe 2
      mulCall.start.astChildren.isIdentifier.l.size shouldBe 2
      mulCall.signature shouldBe "ANY (ANY, ANY)"

      val mulLhs = mulCall.start.astChildren.isIdentifier.l.head
      mulLhs.name shouldBe "tmp"
      mulLhs.typeFullName shouldBe "i32"
      mulLhs.order shouldBe 1
      mulLhs.argumentIndex shouldBe 1
      mulLhs.start.refsTo.l.size shouldBe 1
      mulLhs.start.refsTo.head shouldBe tmp0

      val mulRhs = mulCall.start.astChildren.isIdentifier.l.last
      mulRhs.name shouldBe "tmp1"
      mulRhs.typeFullName shouldBe "i32"
      mulRhs.order shouldBe 2
      mulRhs.argumentIndex shouldBe 2
      mulRhs.start.refsTo.l.size shouldBe 1
      mulRhs.start.refsTo.head shouldBe tmp1
    }

    {
      // ret i32 %0
      val ret = block.start.astChildren.isReturnNode.l.last
      ret.code shouldBe "return"
      ret.order shouldBe 6
      ret.argumentIndex shouldBe 6

      ret.start.astChildren.l.size shouldBe 1
      ret.start.astChildren.isIdentifier.l.size shouldBe 1
      val retVal = ret.start.astChildren.isIdentifier.head

      retVal.name shouldBe "mul"
      retVal.order shouldBe 1
      retVal.argumentIndex shouldBe 1
      retVal.start.refsTo.l.size shouldBe 1
      retVal.start.refsTo.head shouldBe mul
    }

  }

  "CFG" in {
    val method = cpg.method.name(methodName).head
    val block = method.start.block.head

    val assignStoreCall_0 = block.start.astChildren.isCall.l.apply(0)
    val assignStoreCall_1 = block.start.astChildren.isCall.l.apply(1)
    val assignLoadCall_0 = block.start.astChildren.isCall.l.apply(2)
    val assignLoadCall_1 = block.start.astChildren.isCall.l.apply(3)
    val assignMulCall = block.start.astChildren.isCall.l.apply(4)


    {
      // store i32 %x, i32* %x.addr
      val lhs = assignStoreCall_0.start.astChildren.isCall.l.head
      val ref = lhs.start.astChildren.isIdentifier.l.head
      val rhs = assignStoreCall_0.start.astChildren.isIdentifier.l.head

      ref.start.cfgNext.head shouldBe lhs
      lhs.start.cfgNext.head shouldBe rhs
      rhs.start.cfgNext.head shouldBe assignStoreCall_0
    }

    {
      //  store i32 42, i32* %m, align 4
      val lhs = assignStoreCall_1.start.astChildren.isCall.l.head
      val ref = lhs.start.astChildren.isIdentifier.l.head
      val rhs = assignStoreCall_1.start.astChildren.isLiteral.l.head

      ref.start.cfgNext.head shouldBe lhs
      lhs.start.cfgNext.head shouldBe rhs
      rhs.start.cfgNext.head shouldBe assignStoreCall_1
      assignStoreCall_0.start.cfgNext.head shouldBe ref
    }

    {
      // %0 = load i32, i32* %m
      val lhs =  assignLoadCall_0.start.astChildren.isIdentifier.head
      val rhs =  assignLoadCall_0.start.astChildren.isCall.head
      val ref = rhs.start.astChildren.isIdentifier.head

      lhs.start.cfgNext.head shouldBe ref
      ref.start.cfgNext.head shouldBe rhs
      rhs.start.cfgNext.head shouldBe assignLoadCall_0
      assignStoreCall_1.start.cfgNext.head shouldBe lhs
    }

    {
      // %1 = load i32, i32* %x.addr
      val lhs =  assignLoadCall_1.start.astChildren.isIdentifier.head
      val rhs =  assignLoadCall_1.start.astChildren.isCall.head
      val ref = rhs.start.astChildren.isIdentifier.head

      lhs.start.cfgNext.head shouldBe ref
      ref.start.cfgNext.head shouldBe rhs
      rhs.start.cfgNext.head shouldBe assignLoadCall_1
      assignLoadCall_0.start.cfgNext.head shouldBe lhs
    }

    {
      //  %mul = mul nsw i32 %0, %1
      val lhs = assignMulCall.start.astChildren.isIdentifier.head
      val mulCall = assignMulCall.start.astChildren.isCall.head
      val mulLhs = mulCall.start.astChildren.isIdentifier.l.head
      val mulRhs = mulCall.start.astChildren.isIdentifier.l.last

      lhs.start.cfgNext.head shouldBe mulLhs
      mulLhs.start.cfgNext.head shouldBe mulRhs
      mulRhs.start.cfgNext.head shouldBe mulCall
      assignLoadCall_1.start.cfgNext.head shouldBe lhs
    }

    {
      // ret i32 %0
      val ret = block.start.astChildren.isReturnNode.l.last
      val retVal = ret.start.astChildren.isIdentifier.head
      retVal.start.cfgNext.head shouldBe ret
      assignMulCall.start.cfgNext.head shouldBe retVal

      val methodReturn = method.start.methodReturn.head
      methodReturn.start.cfgLast.head shouldBe ret
    }
  }

}
