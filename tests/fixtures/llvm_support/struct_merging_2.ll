%struct.Same = type { i32 }
%struct.TuplePair = type { i32, i32 }

define void @use_same_struct_2(%struct.Same) {
entry:
  ret void
}

define void @use_tuple_pair_struct(%struct.TuplePair) {
entry:
  ret void
}