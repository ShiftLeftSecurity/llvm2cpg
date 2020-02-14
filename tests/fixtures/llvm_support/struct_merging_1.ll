%struct.Same = type { i32 }
%struct.Same.125 = type { i32 }

%struct.Point = type { i32, i32 }
%struct.Point.4D_2 = type { i32, i32, i32, i32 }
%struct.Point.3D = type { i32, i32, i32 }
%struct.Point.4D = type { i32, i32, i32, i32 }

define void @use_same_struct_1(%struct.Same) {
entry:
  ret void
}

define void @use_same_125_struct(%struct.Same.125) {
entry:
  ret void
}

define void @use_point_struct(%struct.Point) {
entry:
  ret void
}

define void @use_point_3d_struct(%struct.Point.3D) {
entry:
  ret void
}

define void @use_point_4d_struct(%struct.Point.4D) {
entry:
  ret void
}

define void @use_point_4d2_struct(%struct.Point.4D_2) {
entry:
  ret void
}