; Generated from basic_c_support/inline_asm.c

define void @inline_asm() {
entry:
  call void asm sideeffect "add %al, (%rax)", "~{dirflag},~{fpsr},~{flags}"()
  ret void
}
