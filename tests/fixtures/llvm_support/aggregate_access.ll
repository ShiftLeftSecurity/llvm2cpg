define {i32, float} @insert(float %val) {
    %agg1 = insertvalue {i32, float} undef, i32 1, 0              ; yields {i32 1, float undef}
    %agg2 = insertvalue {i32, float} %agg1, float %val, 1         ; yields {i32 1, float %val}
    %agg3 = insertvalue {i32, {[ 1 x float]}} undef, float %val, 1, 0, 0
    ret {i32, float} %agg2
}


define void @extract() {
    %A = extractvalue {i32, float} zeroinitializer, 1
    %B = extractvalue {i32, {[ 3 x {i8, i8, i8, [4 x i8]}]}} zeroinitializer, 1, 0, 2, 3, 0
    ret void
}
