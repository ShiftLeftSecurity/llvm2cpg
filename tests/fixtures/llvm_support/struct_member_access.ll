; This example constructed from the XNU kernel bitcode

%struct.kcdata_subtype_descriptor = type { i8, i8, i16, i32, [32 x i8] }
%struct.testtyp = type {i8,{i8,{i8,i8}}}


@kc_xnupost_test_def = global [7 x %struct.kcdata_subtype_descriptor] [
    %struct.kcdata_subtype_descriptor { i8 0, i8 5, i16 0, i32 2, [32 x i8] c"config\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00" },
    %struct.kcdata_subtype_descriptor { i8 0, i8 5, i16 2, i32 2, [32 x i8] c"test_num\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00" },
    %struct.kcdata_subtype_descriptor { i8 0, i8 6, i16 4, i32 4, [32 x i8] c"retval\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00" },
    %struct.kcdata_subtype_descriptor { i8 0, i8 6, i16 8, i32 4, [32 x i8] c"expected_retval\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00" },
    %struct.kcdata_subtype_descriptor { i8 0, i8 9, i16 12, i32 8, [32 x i8] c"begin_time\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00" },
    %struct.kcdata_subtype_descriptor { i8 0, i8 9, i16 20, i32 8, [32 x i8] c"end_time\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00" },
    %struct.kcdata_subtype_descriptor { i8 1, i8 1, i16 28, i32 8650753, [32 x i8] c"test_name\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00" }
]

define i16* @test() {
  %x = getelementptr [7 x %struct.kcdata_subtype_descriptor], [7 x %struct.kcdata_subtype_descriptor]* @kc_xnupost_test_def, i64 0, i64 6, i32 2
  ret i16* %x
}

define i16* @test1(%struct.kcdata_subtype_descriptor* %p) {
  %x = getelementptr %struct.kcdata_subtype_descriptor, %struct.kcdata_subtype_descriptor* %p, i64 0, i32 2
  ret i16* %x
}

define i8* @test2(%struct.testtyp* %p) {
  %x = getelementptr %struct.testtyp, %struct.testtyp* %p, i32 0, i32 1, i32 1, i32 1
  ret i8* %x
}