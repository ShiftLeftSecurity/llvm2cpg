define [4 x i8] @foo(){
  ret [4 x i8] [i8 43, i8 42, i8 0, i8 110]
}

@.str = private unnamed_addr constant [6 x i8] c"hello\00", align 1
@.x = private unnamed_addr constant i8 112, align 1


define [4 x i8*] @constArray(){
  ret [4 x i8*] [i8* undef, i8* undef, i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str, i64 0, i64 0), i8* undef]
}

define [4 x i8] @constString(){
  ret [4 x i8] c"abc\00"
}

define [4 x i8] @constStringNoNull(){
  ret [4 x i8] c"abcd"
}

;define [5 x i8] @constStringBadUTF8(){
;  ret [5 x i8] c"\CA\FE\BA\BE\00"
;}

define [4 x i8] @constStringInternalZero(){
  ret [4 x i8] c"A\00B\00"
}

define [4 x i16] @constDataArray(){
  ret [4 x i16] [i16 1, i16 2, i16 3, i16 4]
}


define <4 x i8> @constDataVec(){
  ret <4 x i8> <i8 1, i8 1, i8 2, i8 3>
}

define <4 x i8> @constVec(){
  ret <4 x i8> <i8 1, i8 undef, i8 2, i8 3>
}

%struct.pointT = type { i32, i32, [2 x i16] }

define { i32, i32, i32 } @constStruct(){
  ret { i32, i32, i32 } {i32 1, i32 2, i32 5}
}
