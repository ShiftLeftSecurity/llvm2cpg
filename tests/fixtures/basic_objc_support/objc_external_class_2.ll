; ModuleID = '/opt/ShiftLeft/llvm2cpg/tests/fixtures/basic_objc_support/objc_external_class_2.m'
source_filename = "/opt/ShiftLeft/llvm2cpg/tests/fixtures/basic_objc_support/objc_external_class_2.m"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.14.0"

%0 = type opaque
%struct._objc_cache = type opaque
%struct._class_t = type { %struct._class_t*, %struct._class_t*, %struct._objc_cache*, i8* (i8*, i8*)**, %struct._class_ro_t* }
%struct._class_ro_t = type { i32, i32, i32, i8*, i8*, %struct.__method_list_t*, %struct._objc_protocol_list*, %struct._ivar_list_t*, i8*, %struct._prop_list_t* }
%struct.__method_list_t = type { i32, i32, [0 x %struct._objc_method] }
%struct._objc_method = type { i8*, i8*, i8* }
%struct._objc_protocol_list = type { i64, [0 x %struct._protocol_t*] }
%struct._protocol_t = type { i8*, i8*, %struct._objc_protocol_list*, %struct.__method_list_t*, %struct.__method_list_t*, %struct.__method_list_t*, %struct.__method_list_t*, %struct._prop_list_t*, i32, i32, i8**, i8*, %struct._prop_list_t* }
%struct._ivar_list_t = type { i32, i32, [0 x %struct._ivar_t] }
%struct._ivar_t = type { i64*, i8*, i8*, i32, i32 }
%struct._prop_list_t = type { i32, i32, [0 x %struct._prop_t] }
%struct._prop_t = type { i8*, i8* }

@_objc_empty_cache = external global %struct._objc_cache
@"OBJC_METACLASS_$_RootClass" = external global %struct._class_t
@OBJC_CLASS_NAME_ = private unnamed_addr constant [7 x i8] c"Child2\00", section "__TEXT,__objc_classname,cstring_literals", align 1
@"_OBJC_METACLASS_RO_$_Child2" = internal global %struct._class_ro_t { i32 1, i32 40, i32 40, i8* null, i8* getelementptr inbounds ([7 x i8], [7 x i8]* @OBJC_CLASS_NAME_, i32 0, i32 0), %struct.__method_list_t* null, %struct._objc_protocol_list* null, %struct._ivar_list_t* null, i8* null, %struct._prop_list_t* null }, section "__DATA, __objc_const", align 8
@"OBJC_METACLASS_$_Child2" = global %struct._class_t { %struct._class_t* @"OBJC_METACLASS_$_RootClass", %struct._class_t* @"OBJC_METACLASS_$_RootClass", %struct._objc_cache* @_objc_empty_cache, i8* (i8*, i8*)** null, %struct._class_ro_t* @"_OBJC_METACLASS_RO_$_Child2" }, section "__DATA, __objc_data", align 8
@"OBJC_CLASS_$_RootClass" = external global %struct._class_t
@"_OBJC_CLASS_RO_$_Child2" = internal global %struct._class_ro_t { i32 0, i32 0, i32 0, i8* null, i8* getelementptr inbounds ([7 x i8], [7 x i8]* @OBJC_CLASS_NAME_, i32 0, i32 0), %struct.__method_list_t* null, %struct._objc_protocol_list* null, %struct._ivar_list_t* null, i8* null, %struct._prop_list_t* null }, section "__DATA, __objc_const", align 8
@"OBJC_CLASS_$_Child2" = global %struct._class_t { %struct._class_t* @"OBJC_METACLASS_$_Child2", %struct._class_t* @"OBJC_CLASS_$_RootClass", %struct._objc_cache* @_objc_empty_cache, i8* (i8*, i8*)** null, %struct._class_ro_t* @"_OBJC_CLASS_RO_$_Child2" }, section "__DATA, __objc_data", align 8
@"OBJC_CLASSLIST_REFERENCES_$_" = internal global %struct._class_t* @"OBJC_CLASS_$_Child2", section "__DATA,__objc_classrefs,regular,no_dead_strip", align 8
@OBJC_METH_VAR_NAME_ = private unnamed_addr constant [4 x i8] c"new\00", section "__TEXT,__objc_methname,cstring_literals", align 1
@OBJC_SELECTOR_REFERENCES_ = internal externally_initialized global i8* getelementptr inbounds ([4 x i8], [4 x i8]* @OBJC_METH_VAR_NAME_, i32 0, i32 0), section "__DATA,__objc_selrefs,literal_pointers,no_dead_strip", align 8
@"OBJC_LABEL_CLASS_$" = internal global [1 x i8*] [i8* bitcast (%struct._class_t* @"OBJC_CLASS_$_Child2" to i8*)], section "__DATA,__objc_classlist,regular,no_dead_strip", align 8
@llvm.compiler.used = appending global [5 x i8*] [i8* getelementptr inbounds ([7 x i8], [7 x i8]* @OBJC_CLASS_NAME_, i32 0, i32 0), i8* bitcast (%struct._class_t** @"OBJC_CLASSLIST_REFERENCES_$_" to i8*), i8* getelementptr inbounds ([4 x i8], [4 x i8]* @OBJC_METH_VAR_NAME_, i32 0, i32 0), i8* bitcast (i8** @OBJC_SELECTOR_REFERENCES_ to i8*), i8* bitcast ([1 x i8*]* @"OBJC_LABEL_CLASS_$" to i8*)], section "llvm.metadata"

; Function Attrs: noinline optnone ssp uwtable
define %0* @newChild() #0 {
entry:
  %0 = load %struct._class_t*, %struct._class_t** @"OBJC_CLASSLIST_REFERENCES_$_", align 8
  %1 = load i8*, i8** @OBJC_SELECTOR_REFERENCES_, align 8, !invariant.load !9
  %2 = bitcast %struct._class_t* %0 to i8*
  %call = call i8* bitcast (i8* (i8*, i8*, ...)* @objc_msgSend to i8* (i8*, i8*)*)(i8* %2, i8* %1)
  %3 = bitcast i8* %call to %0*
  ret %0* %3
}

; Function Attrs: nonlazybind
declare i8* @objc_msgSend(i8*, i8*, ...) #1

attributes #0 = { noinline optnone ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nonlazybind }

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
!8 = !{!"clang version 9.0.0 (git://github.com/llvm/llvm-project.git 0399d5a9682b3cef71c653373e38890c63c4c365)"}
!9 = !{}
