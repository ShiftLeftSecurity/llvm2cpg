define void @switches() {
entry:
  switch i32 14, label %default [
    i32 0, label %first
    i32 1, label %second
  ]

first:                                         ; preds = %entry
  br label %return

second:                                        ; preds = %entry
  br label %return

default:                                       ; preds = %entry
  br label %return

return:                                        ; preds = %default, %first, %second
  ret void
}