define i32 @atomic_inc(i32* %x) {
  %nv = atomicrmw add i32* %x, i32 1 acq_rel
  ret i32 %nv
}

define void @atomiccmpxchg(i32* %ptr) {
  cmpxchg i32* %ptr, i32 42, i32 0 acq_rel monotonic
  ret void
}