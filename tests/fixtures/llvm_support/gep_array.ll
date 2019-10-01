; Generated from basic_c_support/gep_array.c and slightly modified by hand

define i32* @gep_array(i32* %x) {
  %ptr = getelementptr inbounds i32, i32* %x, i64 1  ;  1
  ret i32* %ptr                                      ;  2
}
