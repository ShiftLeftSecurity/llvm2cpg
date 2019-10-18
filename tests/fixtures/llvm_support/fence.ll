define void @foo() { ; taken from llvm langref
  fence acquire
  fence syncscope("singlethread") seq_cst
  fence syncscope("agent") seq_cst
  ret void
}