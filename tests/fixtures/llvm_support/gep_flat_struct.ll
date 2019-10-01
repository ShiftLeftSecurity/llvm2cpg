; Generated from basic_c_support/gep_flat_struct.c and slightly modified by hand

%struct.Point = type { i32, i32 }

define void @flat_struct() {
  %p = alloca %struct.Point                                           ; 0
  %x = getelementptr %struct.Point, %struct.Point* %p, i32 0, i32 0   ; 1
  store i32 127, i32* %x                                              ; 2
  %y = getelementptr %struct.Point, %struct.Point* %p, i32 0, i32 1   ; 3
  store i32 15, i32* %y                                               ; 4
  ret void                                                            ; 5
}
