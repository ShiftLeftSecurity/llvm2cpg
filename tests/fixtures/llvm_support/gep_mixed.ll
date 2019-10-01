; Example taken from the official documentation (and slightly modified):
; https://llvm.org/docs/LangRef.html#getelementptr-instruction

%struct.RT = type { i8, [10 x [20 x i32]], i8 }
%struct.ST = type { i32, double, %struct.RT }

define i32* @gep_mixed(%struct.ST* %s) {
  %index = getelementptr %struct.ST, %struct.ST* %s, i64 4, i32 2, i32 1, i64 5, i64 13 ; 1
  ret i32* %index                                                                       ; 2
}