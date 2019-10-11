define void @empty_branches() {
entry:
  br label %mid

exit:
  ret void

mid:
  br label %exit
}

define void @empty_conditional_branches() {
entry:
  br i1 true, label %true, label %false

true:
  br label %exit

false:
  br label %exit

exit:
  ret void
}

define void @endless_loop() {
entry:
  br label %loop

loop:               ; preds = %loop, %entry
  br label %loop

return:             ; No predecessors!
  ret void
}