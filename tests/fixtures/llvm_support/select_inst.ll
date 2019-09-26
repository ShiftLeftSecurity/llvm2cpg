define i32 @basic_c_support() {
entry:
  %sel = select i1 true, i32 15, i32 17
  ret i32 %sel
}
