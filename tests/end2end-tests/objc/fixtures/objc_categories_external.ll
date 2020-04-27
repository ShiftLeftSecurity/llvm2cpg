; ModuleID = '/opt/ShiftLeft/llvm2cpg/tests/fixtures/basic_objc_support/objc_categories.m'
source_filename = "/opt/ShiftLeft/llvm2cpg/tests/fixtures/basic_objc_support/objc_categories.m"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.15.0"

%0 = type opaque
%struct._class_t = type { %struct._class_t*, %struct._class_t*, %struct._objc_cache*, i8* (i8*, i8*)**, %struct._class_ro_t* }
%struct._objc_cache = type opaque
%struct._class_ro_t = type { i32, i32, i32, i8*, i8*, %struct.__method_list_t*, %struct._objc_protocol_list*, %struct._ivar_list_t*, i8*, %struct._prop_list_t* }
%struct.__method_list_t = type { i32, i32, [0 x %struct._objc_method] }
%struct._objc_method = type { i8*, i8*, i8* }
%struct._objc_protocol_list = type { i64, [0 x %struct._protocol_t*] }
%struct._protocol_t = type { i8*, i8*, %struct._objc_protocol_list*, %struct.__method_list_t*, %struct.__method_list_t*, %struct.__method_list_t*, %struct.__method_list_t*, %struct._prop_list_t*, i32, i32, i8**, i8*, %struct._prop_list_t* }
%struct._ivar_list_t = type { i32, i32, [0 x %struct._ivar_t] }
%struct._ivar_t = type { i64*, i8*, i8*, i32, i32 }
%struct._prop_list_t = type { i32, i32, [0 x %struct._prop_t] }
%struct._prop_t = type { i8*, i8* }
%struct._category_t = type { i8*, %struct._class_t*, %struct.__method_list_t*, %struct.__method_list_t*, %struct._objc_protocol_list*, %struct._prop_list_t*, %struct._prop_list_t*, i32 }

@OBJC_CLASS_NAME_ = private unnamed_addr constant [13 x i8] c"SomeCategory\00", section "__TEXT,__objc_classname,cstring_literals", align 1
@"OBJC_CLASS_$_RootClass" = external global %struct._class_t
@OBJC_METH_VAR_NAME_ = private unnamed_addr constant [12 x i8] c"doSomething\00", section "__TEXT,__objc_methname,cstring_literals", align 1
@OBJC_METH_VAR_TYPE_ = private unnamed_addr constant [8 x i8] c"v16@0:8\00", section "__TEXT,__objc_methtype,cstring_literals", align 1
@"\01l_OBJC_$_CATEGORY_INSTANCE_METHODS_RootClass_$_SomeCategory" = private global { i32, i32, [1 x %struct._objc_method] } { i32 24, i32 1, [1 x %struct._objc_method] [%struct._objc_method { i8* getelementptr inbounds ([12 x i8], [12 x i8]* @OBJC_METH_VAR_NAME_, i32 0, i32 0), i8* getelementptr inbounds ([8 x i8], [8 x i8]* @OBJC_METH_VAR_TYPE_, i32 0, i32 0), i8* bitcast (void (%0*, i8*)* @"\01-[RootClass(SomeCategory) doSomething]" to i8*) }] }, section "__DATA, __objc_const", align 8
@OBJC_METH_VAR_NAME_.1 = private unnamed_addr constant [16 x i8] c"doSomethingElse\00", section "__TEXT,__objc_methname,cstring_literals", align 1
@"\01l_OBJC_$_CATEGORY_CLASS_METHODS_RootClass_$_SomeCategory" = private global { i32, i32, [1 x %struct._objc_method] } { i32 24, i32 1, [1 x %struct._objc_method] [%struct._objc_method { i8* getelementptr inbounds ([16 x i8], [16 x i8]* @OBJC_METH_VAR_NAME_.1, i32 0, i32 0), i8* getelementptr inbounds ([8 x i8], [8 x i8]* @OBJC_METH_VAR_TYPE_, i32 0, i32 0), i8* bitcast (void (i8*, i8*)* @"\01+[RootClass(SomeCategory) doSomethingElse]" to i8*) }] }, section "__DATA, __objc_const", align 8
@"\01l_OBJC_$_CATEGORY_RootClass_$_SomeCategory" = private global %struct._category_t { i8* getelementptr inbounds ([13 x i8], [13 x i8]* @OBJC_CLASS_NAME_, i32 0, i32 0), %struct._class_t* @"OBJC_CLASS_$_RootClass", %struct.__method_list_t* bitcast ({ i32, i32, [1 x %struct._objc_method] }* @"\01l_OBJC_$_CATEGORY_INSTANCE_METHODS_RootClass_$_SomeCategory" to %struct.__method_list_t*), %struct.__method_list_t* bitcast ({ i32, i32, [1 x %struct._objc_method] }* @"\01l_OBJC_$_CATEGORY_CLASS_METHODS_RootClass_$_SomeCategory" to %struct.__method_list_t*), %struct._objc_protocol_list* null, %struct._prop_list_t* null, %struct._prop_list_t* null, i32 64 }, section "__DATA, __objc_const", align 8
@"OBJC_CLASSLIST_REFERENCES_$_" = private global %struct._class_t* @"OBJC_CLASS_$_RootClass", section "__DATA,__objc_classrefs,regular,no_dead_strip", align 8
@OBJC_SELECTOR_REFERENCES_ = private externally_initialized global i8* getelementptr inbounds ([16 x i8], [16 x i8]* @OBJC_METH_VAR_NAME_.1, i32 0, i32 0), section "__DATA,__objc_selrefs,literal_pointers,no_dead_strip", align 8
@OBJC_SELECTOR_REFERENCES_.2 = private externally_initialized global i8* getelementptr inbounds ([12 x i8], [12 x i8]* @OBJC_METH_VAR_NAME_, i32 0, i32 0), section "__DATA,__objc_selrefs,literal_pointers,no_dead_strip", align 8
@"OBJC_LABEL_CATEGORY_$" = private global [1 x i8*] [i8* bitcast (%struct._category_t* @"\01l_OBJC_$_CATEGORY_RootClass_$_SomeCategory" to i8*)], section "__DATA,__objc_catlist,regular,no_dead_strip", align 8
@llvm.compiler.used = appending global [11 x i8*] [i8* getelementptr inbounds ([13 x i8], [13 x i8]* @OBJC_CLASS_NAME_, i32 0, i32 0), i8* getelementptr inbounds ([12 x i8], [12 x i8]* @OBJC_METH_VAR_NAME_, i32 0, i32 0), i8* getelementptr inbounds ([8 x i8], [8 x i8]* @OBJC_METH_VAR_TYPE_, i32 0, i32 0), i8* bitcast ({ i32, i32, [1 x %struct._objc_method] }* @"\01l_OBJC_$_CATEGORY_INSTANCE_METHODS_RootClass_$_SomeCategory" to i8*), i8* getelementptr inbounds ([16 x i8], [16 x i8]* @OBJC_METH_VAR_NAME_.1, i32 0, i32 0), i8* bitcast ({ i32, i32, [1 x %struct._objc_method] }* @"\01l_OBJC_$_CATEGORY_CLASS_METHODS_RootClass_$_SomeCategory" to i8*), i8* bitcast (%struct._category_t* @"\01l_OBJC_$_CATEGORY_RootClass_$_SomeCategory" to i8*), i8* bitcast (%struct._class_t** @"OBJC_CLASSLIST_REFERENCES_$_" to i8*), i8* bitcast (i8** @OBJC_SELECTOR_REFERENCES_ to i8*), i8* bitcast (i8** @OBJC_SELECTOR_REFERENCES_.2 to i8*), i8* bitcast ([1 x i8*]* @"OBJC_LABEL_CATEGORY_$" to i8*)], section "llvm.metadata"

; Function Attrs: noinline optnone ssp uwtable
define internal void @"\01-[RootClass(SomeCategory) doSomething]"(%0*, i8*) #0 {
  %3 = alloca %0*, align 8
  %4 = alloca i8*, align 8
  store %0* %0, %0** %3, align 8
  store i8* %1, i8** %4, align 8
  ret void
}

; Function Attrs: noinline optnone ssp uwtable
define internal void @"\01+[RootClass(SomeCategory) doSomethingElse]"(i8*, i8*) #0 {
  %3 = alloca i8*, align 8
  %4 = alloca i8*, align 8
  store i8* %0, i8** %3, align 8
  store i8* %1, i8** %4, align 8
  ret void
}

; Function Attrs: noinline optnone ssp uwtable
define %0* @use() #1 {
  %1 = alloca %0*, align 8
  %2 = load %struct._class_t*, %struct._class_t** @"OBJC_CLASSLIST_REFERENCES_$_", align 8
  %3 = bitcast %struct._class_t* %2 to i8*
  %4 = call i8* @objc_alloc_init(i8* %3)
  %5 = bitcast i8* %4 to %0*
  store %0* %5, %0** %1, align 8
  %6 = load %struct._class_t*, %struct._class_t** @"OBJC_CLASSLIST_REFERENCES_$_", align 8
  %7 = load i8*, i8** @OBJC_SELECTOR_REFERENCES_, align 8, !invariant.load !9
  %8 = bitcast %struct._class_t* %6 to i8*
  call void bitcast (i8* (i8*, i8*, ...)* @objc_msgSend to void (i8*, i8*)*)(i8* %8, i8* %7)
  %9 = load %0*, %0** %1, align 8
  %10 = load i8*, i8** @OBJC_SELECTOR_REFERENCES_.2, align 8, !invariant.load !9
  %11 = bitcast %0* %9 to i8*
  call void bitcast (i8* (i8*, i8*, ...)* @objc_msgSend to void (i8*, i8*)*)(i8* %11, i8* %10)
  %12 = load %0*, %0** %1, align 8
  ret %0* %12
}

declare i8* @objc_alloc_init(i8*)

; Function Attrs: nonlazybind
declare i8* @objc_msgSend(i8*, i8*, ...) #2

attributes #0 = { noinline optnone ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline optnone ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "darwin-stkchk-strong-link" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "probe-stack"="___chkstk_darwin" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nonlazybind }

!llvm.module.flags = !{!0, !1, !2, !3, !4, !5, !6, !7}
!llvm.ident = !{!8}

!0 = !{i32 2, !"SDK Version", [2 x i32] [i32 10, i32 15]}
!1 = !{i32 1, !"Objective-C Version", i32 2}
!2 = !{i32 1, !"Objective-C Image Info Version", i32 0}
!3 = !{i32 1, !"Objective-C Image Info Section", !"__DATA,__objc_imageinfo,regular,no_dead_strip"}
!4 = !{i32 4, !"Objective-C Garbage Collection", i32 0}
!5 = !{i32 1, !"Objective-C Class Properties", i32 64}
!6 = !{i32 1, !"wchar_size", i32 4}
!7 = !{i32 7, !"PIC Level", i32 2}
!8 = !{!"Apple clang version 11.0.0 (clang-1100.0.33.12)"}
!9 = !{}
