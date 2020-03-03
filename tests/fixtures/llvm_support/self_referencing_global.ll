@SelfRef = internal global i8* bitcast (i8** @SelfRef to i8*)

define i8** @main() {
  ret i8** @SelfRef
}