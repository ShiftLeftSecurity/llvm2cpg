@x = global i32 0
@y = alias i32, i32* @x

define i32* @aliases() {
  ret i32* @y
}
