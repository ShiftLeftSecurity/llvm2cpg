@.str = private unnamed_addr constant [11 x i8] c"0123456789\00"

define void @store_const() {
  store i8 42, i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str, i64 0, i64 0)
  ret void
}