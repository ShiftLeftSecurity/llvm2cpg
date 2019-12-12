@.str = private unnamed_addr constant [12 x i8] c"\06\09`\86H\01e\03\04\02\01\00"

declare i32 @printstuff(i8*)

define void @basic_c_support() {
entry:
  %call = call i32 @printstuff(i8* getelementptr inbounds ([12 x i8], [12 x i8]* @.str, i32 0, i32 0))
  ret void
}