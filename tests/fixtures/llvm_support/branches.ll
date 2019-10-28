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

define i8 @cfg_conditional(i8 %arg){
entry:
  %y = trunc i8 %arg to i1
  %z = zext i1 %y to i8
  br i1 %y, label %ret1, label %ret2
ret1:
  ret i8 1
ret2:
  ret i8 2
}

define i8 @indirect_branch(i1 %arg){
entry:
  %target = select i1 %arg, i8* blockaddress(@indirect_branch, %ret1), i8* blockaddress(@indirect_branch, %ret2)
  indirectbr i8* %target, [label %ret2, label %ret1]
ret1:
  ret i8 1
ret2:
  ret i8 2
}