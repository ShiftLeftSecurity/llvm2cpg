declare extern_weak i32 @__pthread_key_create(i32*, void (i8*)*)

define i32 @_ZL18__gthread_active_pv() {
  ret i32 zext (i1 icmp ne (i8* bitcast (i32 (i32*, void (i8*)*)* @__pthread_key_create to i8*), i8* null) to i32)
}
