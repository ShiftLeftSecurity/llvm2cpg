define void @unreachable() {
entry:
  br label %mid

mid:
  unreachable

exit:
  ret void
}