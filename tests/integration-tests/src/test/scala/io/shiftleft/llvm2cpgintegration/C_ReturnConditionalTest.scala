package io.shiftleft.llvm2cpgintegration

import io.shiftleft.codepropertygraph.cpgloading.CpgLoader
import io.shiftleft.semanticcpg.language._

class C_ReturnConditionalTest extends CPGMatcher {
  private val cpg = CpgLoader.load(TestCpgPaths.C_ReturnConditionalCPG)
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
    validateTypes(cpg, Set("ANY", "i32", "i32*", "i1"))
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

    val retval = locals.head
    retval.name shouldBe "retval"
    retval.typeFullName shouldBe "i32*"
    retval.order shouldBe 0

    val xaddr = locals.apply(1)
    xaddr.name shouldBe "x.addr"
    xaddr.typeFullName shouldBe "i32*"
    xaddr.order shouldBe 1

    val tmp0 = locals.apply(2)
    tmp0.name shouldBe "tmp"
    tmp0.typeFullName shouldBe "i32"
    tmp0.order shouldBe 2

    val tobool = locals.apply(3)
    tobool.name shouldBe "tobool"
    tobool.typeFullName shouldBe "i1"
    tobool.order shouldBe 3

    val tmp1 = locals.apply(4)
    tmp1.name shouldBe "tmp1"
    tmp1.typeFullName shouldBe "i32"
    tmp1.order shouldBe 4
  }

  /*
entry:
  store i32 %x, i32* %x.addr, align 4
  %0 = load i32, i32* %x.addr, align 4
  %tobool = icmp ne i32 %0, 0
  br i1 %tobool, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  store i32 42, i32* %retval, align 4
  br label %return

if.end:                                           ; preds = %entry
  store i32 36, i32* %retval, align 4
  br label %return

return:                                           ; preds = %if.end, %if.then
  %1 = load i32, i32* %retval, align 4
  ret i32 %1
  */

  "method AST" in {
    val method = cpg.method.name(methodName).head
    val block = method.start.block.head
    block.start.astChildren.l.size shouldBe 14
    block.start.astChildren.isCall.l.size shouldBe 8
    block.start.astChildren.isReturnNode.l.size shouldBe 1

    val param = method.start.parameter.head
    val locals = block.start.local.l
    val retval = locals.head
    val xaddr = locals.apply(1)
    val tmp0 = locals.apply(2)
    val tobool = locals.apply(3)
    val tmp1 = locals.apply(4)

    {
      // %retval = alloca i32, align 4
      val assignCall = block.start.astChildren.isCall.l.head
      assignCall.name shouldBe "<operator>.assignment"
      assignCall.order shouldBe 1
      assignCall.argumentIndex shouldBe 1
      assignCall.typeFullName shouldBe "i32*"

      assignCall.start.astChildren.isIdentifier.l.size shouldBe 1
      val lhs = assignCall.start.astChildren.isIdentifier.head
      lhs.name shouldBe "retval"
      lhs.start.refsTo.l.size shouldBe 1
      lhs.start.refsTo.head shouldBe retval
      lhs.order shouldBe 1
      lhs.argumentIndex shouldBe 1

      assignCall.start.astChildren.isCall.l.size shouldBe 1
      val rhs = assignCall.start.astChildren.isCall.head
      rhs.name shouldBe "<operator>.alloca"
      rhs.methodFullName shouldBe "<operator>.alloca"
      rhs.typeFullName shouldBe "i32*"
      rhs.order shouldBe 2
      rhs.argumentIndex shouldBe 2
    }

    {
      // %x.addr = alloca i32
      val assignCall = block.start.astChildren.isCall.l.apply(1)
      assignCall.name shouldBe "<operator>.assignment"
      assignCall.order shouldBe 2
      assignCall.argumentIndex shouldBe 2
      assignCall.typeFullName shouldBe "i32*"

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
      rhs.typeFullName shouldBe "i32*"
      rhs.order shouldBe 2
      rhs.argumentIndex shouldBe 2
    }

    {
      // store i32 %x, i32* %x.addr
      val assignCall = block.start.astChildren.isCall.l.apply(2)
      assignCall.name shouldBe "<operator>.assignment"
      assignCall.order shouldBe 3
      assignCall.argumentIndex shouldBe 3
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
      // %0 = load i32, i32* %x.addr
      val assignCall = block.start.astChildren.isCall.l.apply(3)
      assignCall.name shouldBe "<operator>.assignment"
      assignCall.order shouldBe 4
      assignCall.argumentIndex shouldBe 4
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
      dereference.name shouldBe "x.addr"
      dereference.typeFullName shouldBe "i32*"
      dereference.order shouldBe 1
      dereference.argumentIndex shouldBe 1
      dereference.start.refsTo.l.size shouldBe 1
      dereference.start.refsTo.head shouldBe xaddr
    }

    {
      //  %tobool = icmp ne i32 %0, 0
      val assignCall = block.start.astChildren.isCall.l.apply(4)
      assignCall.name shouldBe "<operator>.assignment"
      assignCall.order shouldBe 5
      assignCall.argumentIndex shouldBe 5
      assignCall.start.astChildren.l.size shouldBe 2

      assignCall.start.astChildren.isIdentifier.l.size shouldBe 1
      val lhs = assignCall.start.astChildren.isIdentifier.head
      lhs.name shouldBe "tobool"
      lhs.typeFullName shouldBe "i1"
      lhs.start.refsTo.l.size shouldBe 1
      lhs.start.refsTo.head shouldBe tobool
      lhs.order shouldBe 1
      lhs.argumentIndex shouldBe 1

      assignCall.start.astChildren.isCall.l.size shouldBe 1
      val icmpCall = assignCall.start.astChildren.isCall.head
      icmpCall.name shouldBe "<operator>.notEquals"
      icmpCall.methodFullName shouldBe "<operator>.notEquals"
      icmpCall.typeFullName shouldBe "i1"
      icmpCall.order shouldBe 2
      icmpCall.argumentIndex shouldBe 2
      icmpCall.start.astChildren.l.size shouldBe 2

      val icmpLhs = icmpCall.start.astChildren.isIdentifier.l.head
      icmpLhs.name shouldBe "tmp"
      icmpLhs.typeFullName shouldBe "i32"
      icmpLhs.order shouldBe 1
      icmpLhs.argumentIndex shouldBe 1
      icmpLhs.start.refsTo.l.size shouldBe 1
      icmpLhs.start.refsTo.head shouldBe tmp0

      val icmpRhs = icmpCall.start.astChildren.isLiteral.l.last
      icmpRhs.code shouldBe "0"
      icmpRhs.typeFullName shouldBe "i32"
      icmpRhs.order shouldBe 2
      icmpRhs.argumentIndex shouldBe 2
    }

    {
      // store i32 42, i32* %retval
      val assignCall = block.start.astChildren.isCall.l.apply(5)
      assignCall.name shouldBe "<operator>.assignment"
      assignCall.order shouldBe 6
      assignCall.argumentIndex shouldBe 6
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
      indirection.name shouldBe "retval"
      indirection.start.refsTo.head shouldBe retval
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
      // store i32 36, i32* %retval
      val assignCall = block.start.astChildren.isCall.l.apply(6)
      assignCall.name shouldBe "<operator>.assignment"
      assignCall.order shouldBe 7
      assignCall.argumentIndex shouldBe 7
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
      indirection.name shouldBe "retval"
      indirection.start.refsTo.head shouldBe retval
      indirection.order shouldBe 1
      indirection.argumentIndex shouldBe 1

      assignCall.start.astChildren.isLiteral.l.size shouldBe 1
      val rhs = assignCall.start.astChildren.isLiteral.l.head
      rhs.code shouldBe "36"
      rhs.typeFullName shouldBe "i32"
      rhs.order shouldBe 2
      rhs.argumentIndex shouldBe 2
    }

    {
      //  %1 = load i32, i32* %retval
      val assignCall = block.start.astChildren.isCall.l.apply(7)
      assignCall.name shouldBe "<operator>.assignment"
      assignCall.order shouldBe 8
      assignCall.argumentIndex shouldBe 8
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
      dereference.name shouldBe "retval"
      dereference.typeFullName shouldBe "i32*"
      dereference.order shouldBe 1
      dereference.argumentIndex shouldBe 1
      dereference.start.refsTo.l.size shouldBe 1
      dereference.start.refsTo.head shouldBe retval
    }

    {
      // ret i32 %0
      val ret = block.start.astChildren.isReturnNode.l.last
      ret.code shouldBe "return"
      ret.order shouldBe 9
      ret.argumentIndex shouldBe 9

      ret.start.astChildren.l.size shouldBe 1
      ret.start.astChildren.isIdentifier.l.size shouldBe 1
      val retVal = ret.start.astChildren.isIdentifier.head

      retVal.name shouldBe "tmp1"
      retVal.order shouldBe 1
      retVal.argumentIndex shouldBe 1
      retVal.start.refsTo.l.size shouldBe 1
      retVal.start.refsTo.head shouldBe tmp1
    }

  }

  "CFG" in {
    val method = cpg.method.name(methodName).head
    val block = method.start.block.head

    val assignRetvalAlloca = block.start.astChildren.isCall.l.head
    val assignXaddrAlloca = block.start.astChildren.isCall.l.apply(1)
    val assignXaddrX = block.start.astChildren.isCall.l.apply(2)
    val assignLoadXaddr = block.start.astChildren.isCall.l.apply(3)
    val assignIcmp = block.start.astChildren.isCall.l.apply(4)
    val assignStoreRetval42 = block.start.astChildren.isCall.l.apply(5)
    val assignStoreRetval36 = block.start.astChildren.isCall.l.apply(6)
    val assignLoadRetval = block.start.astChildren.isCall.l.apply(7)
    val ret = block.start.astChildren.isReturnNode.l.last

    {
      // %retval = alloca i32, align 4
      val lhs = assignRetvalAlloca.start.astChildren.isIdentifier.head
      val rhs = assignRetvalAlloca.start.astChildren.isCall.head

      lhs.start.cfgNext.head shouldBe rhs
      rhs.start.cfgNext.head shouldBe assignRetvalAlloca
    }

    {
      // %x.addr = alloca i32
      val lhs = assignXaddrAlloca.start.astChildren.isIdentifier.head
      val rhs = assignXaddrAlloca.start.astChildren.isCall.head

      lhs.start.cfgNext.head shouldBe rhs
      rhs.start.cfgNext.head shouldBe assignXaddrAlloca
      assignRetvalAlloca.start.cfgNext.head shouldBe lhs
    }

    {
      // store i32 %x, i32* %x.addr
      val lhs = assignXaddrX.start.astChildren.isCall.l.head
      val ref = lhs.start.astChildren.isIdentifier.l.head
      val rhs = assignXaddrX.start.astChildren.isIdentifier.l.head

      ref.start.cfgNext.head shouldBe lhs
      lhs.start.cfgNext.head shouldBe rhs
      rhs.start.cfgNext.head shouldBe assignXaddrX
      assignXaddrAlloca.start.cfgNext.head shouldBe ref
    }

    {
      // %0 = load i32, i32* %x.addr
      val lhs = assignLoadXaddr.start.astChildren.isIdentifier.head
      val rhs = assignLoadXaddr.start.astChildren.isCall.head
      val ref = rhs.start.astChildren.isIdentifier.head
      lhs.start.cfgNext.head shouldBe ref
      ref.start.cfgNext.head shouldBe rhs
      rhs.start.cfgNext.head shouldBe assignLoadXaddr
      assignXaddrX.start.cfgNext.head shouldBe lhs
    }

    {
      //  %tobool = icmp ne i32 %0, 0
      val lhs = assignIcmp.start.astChildren.isIdentifier.head
      val icmpCall = assignIcmp.start.astChildren.isCall.head
      val icmpLhs = icmpCall.start.astChildren.isIdentifier.l.head
      val icmpRhs = icmpCall.start.astChildren.isLiteral.l.last

      lhs.start.cfgNext.head shouldBe icmpLhs
      icmpLhs.start.cfgNext.head shouldBe icmpRhs
      icmpRhs.start.cfgNext.head shouldBe icmpCall
      icmpCall.start.cfgNext.head shouldBe assignIcmp
      assignLoadXaddr.start.cfgNext.head shouldBe lhs
    }

    {
      // store i32 42, i32* %retval
      val lhs42 = assignStoreRetval42.start.astChildren.isCall.l.head
      val ref42 = lhs42.start.astChildren.isIdentifier.l.head
      val rhs42 = assignStoreRetval42.start.astChildren.isLiteral.l.head

      ref42.start.cfgNext.head shouldBe lhs42
      lhs42.start.cfgNext.head shouldBe rhs42
      rhs42.start.cfgNext.head shouldBe assignStoreRetval42

      // store i32 36, i32* %retval
      val lhs36 = assignStoreRetval36.start.astChildren.isCall.l.head
      val ref36 = lhs36.start.astChildren.isIdentifier.l.head
      val rhs36 = assignStoreRetval36.start.astChildren.isLiteral.l.head

      ref36.start.cfgNext.head shouldBe lhs36
      lhs36.start.cfgNext.head shouldBe rhs36
      rhs36.start.cfgNext.head shouldBe assignStoreRetval36

      assignIcmp.start.cfgNext.l.size shouldBe 2
      assignIcmp.start.cfgNext.toSet shouldBe Set(ref42, ref36)
    }

    {
      //  %1 = load i32, i32* %retval
      val lhs = assignLoadRetval.start.astChildren.isIdentifier.head
      val rhs = assignLoadRetval.start.astChildren.isCall.head
      val ref = rhs.start.astChildren.isIdentifier.head

      lhs.start.cfgNext.head shouldBe ref
      ref.start.cfgNext.head shouldBe rhs
      rhs.start.cfgNext.head shouldBe assignLoadRetval

      assignStoreRetval36.start.cfgNext.head shouldBe lhs
      assignStoreRetval42.start.cfgNext.head shouldBe lhs
    }

    {
      // ret i32 %0

      val retVal = ret.start.astChildren.isIdentifier.head
      retVal.start.cfgNext.head shouldBe ret
      assignLoadRetval.start.cfgNext.head shouldBe retVal

      val methodReturn = method.start.methodReturn.head
      methodReturn.start.cfgLast.head shouldBe ret
    }
  }

}
