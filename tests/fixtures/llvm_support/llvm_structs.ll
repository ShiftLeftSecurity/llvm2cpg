%struct.ColorA = type { %struct.Color, float }
%struct.Color = type { i32, i32, i32 }
%struct.Poly = type { i32, %union.anon }
%union.anon = type { double }

define void @use(i64, i64, i32, i64) {
  %5 = alloca %struct.ColorA, align 4
  %6 = alloca %struct.Poly, align 8
  %7 = bitcast %struct.ColorA* %5 to { i64, i64 }*
  %8 = getelementptr inbounds { i64, i64 }, { i64, i64 }* %7, i32 0, i32 0
  store i64 %0, i64* %8, align 4
  %9 = getelementptr inbounds { i64, i64 }, { i64, i64 }* %7, i32 0, i32 1
  store i64 %1, i64* %9, align 4
  %10 = bitcast %struct.Poly* %6 to { i32, i64 }*
  %11 = getelementptr inbounds { i32, i64 }, { i32, i64 }* %10, i32 0, i32 0
  store i32 %2, i32* %11, align 8
  %12 = getelementptr inbounds { i32, i64 }, { i32, i64 }* %10, i32 0, i32 1
  store i64 %3, i64* %12, align 8
  ret void
}