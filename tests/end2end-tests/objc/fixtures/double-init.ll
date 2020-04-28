; ModuleID = 'main.m'
source_filename = "main.m"
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
%struct._objc_super = type { i8*, i8* }

@"OBJC_CLASS_$_FooClass" = global %struct._class_t { %struct._class_t* @"OBJC_METACLASS_$_FooClass", %struct._class_t* @"OBJC_CLASS_$_NSObject", %struct._objc_cache* @_objc_empty_cache, i8* (i8*, i8*)** null, %struct._class_ro_t* @"\01l_OBJC_CLASS_RO_$_FooClass" }, section "__DATA, __objc_data", align 8
@"OBJC_CLASSLIST_SUP_REFS_$_" = private global %struct._class_t* @"OBJC_CLASS_$_FooClass", section "__DATA,__objc_superrefs,regular,no_dead_strip", align 8
@OBJC_METH_VAR_NAME_ = private unnamed_addr constant [5 x i8] c"init\00", section "__TEXT,__objc_methname,cstring_literals", align 1
@OBJC_SELECTOR_REFERENCES_ = private externally_initialized global i8* getelementptr inbounds ([5 x i8], [5 x i8]* @OBJC_METH_VAR_NAME_, i32 0, i32 0), section "__DATA,__objc_selrefs,literal_pointers,no_dead_strip", align 8
@_objc_empty_cache = external global %struct._objc_cache
@"OBJC_METACLASS_$_NSObject" = external global %struct._class_t
@OBJC_CLASS_NAME_ = private unnamed_addr constant [9 x i8] c"FooClass\00", section "__TEXT,__objc_classname,cstring_literals", align 1
@"\01l_OBJC_METACLASS_RO_$_FooClass" = private global %struct._class_ro_t { i32 1, i32 40, i32 40, i8* null, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @OBJC_CLASS_NAME_, i32 0, i32 0), %struct.__method_list_t* null, %struct._objc_protocol_list* null, %struct._ivar_list_t* null, i8* null, %struct._prop_list_t* null }, section "__DATA, __objc_const", align 8
@"OBJC_METACLASS_$_FooClass" = global %struct._class_t { %struct._class_t* @"OBJC_METACLASS_$_NSObject", %struct._class_t* @"OBJC_METACLASS_$_NSObject", %struct._objc_cache* @_objc_empty_cache, i8* (i8*, i8*)** null, %struct._class_ro_t* @"\01l_OBJC_METACLASS_RO_$_FooClass" }, section "__DATA, __objc_data", align 8
@"OBJC_CLASS_$_NSObject" = external global %struct._class_t
@OBJC_METH_VAR_NAME_.1 = private unnamed_addr constant [22 x i8] c"initWithBytes:length:\00", section "__TEXT,__objc_methname,cstring_literals", align 1
@OBJC_METH_VAR_TYPE_ = private unnamed_addr constant [16 x i8] c"@28@0:8r^v16i24\00", section "__TEXT,__objc_methtype,cstring_literals", align 1
@"\01l_OBJC_$_INSTANCE_METHODS_FooClass" = private global { i32, i32, [1 x %struct._objc_method] } { i32 24, i32 1, [1 x %struct._objc_method] [%struct._objc_method { i8* getelementptr inbounds ([22 x i8], [22 x i8]* @OBJC_METH_VAR_NAME_.1, i32 0, i32 0), i8* getelementptr inbounds ([16 x i8], [16 x i8]* @OBJC_METH_VAR_TYPE_, i32 0, i32 0), i8* bitcast (i8* (%0*, i8*, i8*, i32)* @"\01-[FooClass initWithBytes:length:]" to i8*) }] }, section "__DATA, __objc_const", align 8
@"\01l_OBJC_CLASS_RO_$_FooClass" = private global %struct._class_ro_t { i32 0, i32 8, i32 8, i8* null, i8* getelementptr inbounds ([9 x i8], [9 x i8]* @OBJC_CLASS_NAME_, i32 0, i32 0), %struct.__method_list_t* bitcast ({ i32, i32, [1 x %struct._objc_method] }* @"\01l_OBJC_$_INSTANCE_METHODS_FooClass" to %struct.__method_list_t*), %struct._objc_protocol_list* null, %struct._ivar_list_t* null, i8* null, %struct._prop_list_t* null }, section "__DATA, __objc_const", align 8
@"OBJC_CLASSLIST_REFERENCES_$_" = private global %struct._class_t* @"OBJC_CLASS_$_FooClass", section "__DATA,__objc_classrefs,regular,no_dead_strip", align 8
@OBJC_SELECTOR_REFERENCES_.2 = private externally_initialized global i8* getelementptr inbounds ([22 x i8], [22 x i8]* @OBJC_METH_VAR_NAME_.1, i32 0, i32 0), section "__DATA,__objc_selrefs,literal_pointers,no_dead_strip", align 8
@"OBJC_LABEL_CLASS_$" = private global [1 x i8*] [i8* bitcast (%struct._class_t* @"OBJC_CLASS_$_FooClass" to i8*)], section "__DATA,__objc_classlist,regular,no_dead_strip", align 8
@llvm.compiler.used = appending global [10 x i8*] [i8* bitcast (%struct._class_t** @"OBJC_CLASSLIST_SUP_REFS_$_" to i8*), i8* getelementptr inbounds ([5 x i8], [5 x i8]* @OBJC_METH_VAR_NAME_, i32 0, i32 0), i8* bitcast (i8** @OBJC_SELECTOR_REFERENCES_ to i8*), i8* getelementptr inbounds ([9 x i8], [9 x i8]* @OBJC_CLASS_NAME_, i32 0, i32 0), i8* getelementptr inbounds ([22 x i8], [22 x i8]* @OBJC_METH_VAR_NAME_.1, i32 0, i32 0), i8* getelementptr inbounds ([16 x i8], [16 x i8]* @OBJC_METH_VAR_TYPE_, i32 0, i32 0), i8* bitcast ({ i32, i32, [1 x %struct._objc_method] }* @"\01l_OBJC_$_INSTANCE_METHODS_FooClass" to i8*), i8* bitcast (%struct._class_t** @"OBJC_CLASSLIST_REFERENCES_$_" to i8*), i8* bitcast (i8** @OBJC_SELECTOR_REFERENCES_.2 to i8*), i8* bitcast ([1 x i8*]* @"OBJC_LABEL_CLASS_$" to i8*)], section "llvm.metadata"

; Function Attrs: noinline optnone ssp uwtable
define internal i8* @"\01-[FooClass initWithBytes:length:]"(%0*, i8*, i8*, i32) #0 !dbg !25 {
  %5 = alloca %0*, align 8
  %6 = alloca i8*, align 8
  %7 = alloca i8*, align 8
  %8 = alloca i32, align 4
  %9 = alloca %struct._objc_super, align 8
  store %0* %0, %0** %5, align 8
  call void @llvm.dbg.declare(metadata %0** %5, metadata !36, metadata !DIExpression()), !dbg !37
  store i8* %1, i8** %6, align 8
  call void @llvm.dbg.declare(metadata i8** %6, metadata !38, metadata !DIExpression()), !dbg !37
  store i8* %2, i8** %7, align 8
  call void @llvm.dbg.declare(metadata i8** %7, metadata !40, metadata !DIExpression()), !dbg !41
  store i32 %3, i32* %8, align 4
  call void @llvm.dbg.declare(metadata i32* %8, metadata !42, metadata !DIExpression()), !dbg !43
  %10 = load %0*, %0** %5, align 8, !dbg !44
  %11 = bitcast %0* %10 to i8*, !dbg !44
  %12 = getelementptr inbounds %struct._objc_super, %struct._objc_super* %9, i32 0, i32 0, !dbg !44
  store i8* %11, i8** %12, align 8, !dbg !44
  %13 = load %struct._class_t*, %struct._class_t** @"OBJC_CLASSLIST_SUP_REFS_$_", align 8, !dbg !44
  %14 = bitcast %struct._class_t* %13 to i8*, !dbg !44
  %15 = getelementptr inbounds %struct._objc_super, %struct._objc_super* %9, i32 0, i32 1, !dbg !44
  store i8* %14, i8** %15, align 8, !dbg !44
  %16 = load i8*, i8** @OBJC_SELECTOR_REFERENCES_, align 8, !dbg !44, !invariant.load !12
  %17 = call i8* bitcast (i8* (%struct._objc_super*, i8*, ...)* @objc_msgSendSuper2 to i8* (%struct._objc_super*, i8*)*)(%struct._objc_super* %9, i8* %16), !dbg !44
  %18 = bitcast i8* %17 to %0*, !dbg !44
  store %0* %18, %0** %5, align 8, !dbg !45
  %19 = load %0*, %0** %5, align 8, !dbg !46
  %20 = bitcast %0* %19 to i8*, !dbg !46
  ret i8* %20, !dbg !47
}

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

declare i8* @objc_msgSendSuper2(%struct._objc_super*, i8*, ...)

; Function Attrs: noinline optnone ssp uwtable
define i32 @main() #2 !dbg !48 {
  %1 = alloca i32, align 4
  %2 = alloca i8*, align 8
  %3 = alloca %0*, align 8
  %4 = alloca %0*, align 8
  store i32 0, i32* %1, align 4
  call void @llvm.dbg.declare(metadata i8** %2, metadata !51, metadata !DIExpression()), !dbg !52
  store i8* null, i8** %2, align 8, !dbg !52
  call void @llvm.dbg.declare(metadata %0** %3, metadata !53, metadata !DIExpression()), !dbg !54
  %5 = load %struct._class_t*, %struct._class_t** @"OBJC_CLASSLIST_REFERENCES_$_", align 8, !dbg !55
  %6 = bitcast %struct._class_t* %5 to i8*, !dbg !55
  %7 = call i8* @objc_alloc(i8* %6), !dbg !55
  %8 = bitcast i8* %7 to %0*, !dbg !55
  %9 = load i8*, i8** %2, align 8, !dbg !56
  %10 = load i8*, i8** @OBJC_SELECTOR_REFERENCES_.2, align 8, !dbg !57, !invariant.load !12
  %11 = bitcast %0* %8 to i8*, !dbg !57
  %12 = call i8* bitcast (i8* (i8*, i8*, ...)* @objc_msgSend to i8* (i8*, i8*, i8*, i32)*)(i8* %11, i8* %10, i8* %9, i32 0), !dbg !57
  %13 = bitcast i8* %12 to %0*, !dbg !57
  store %0* %13, %0** %3, align 8, !dbg !54
  %14 = load %0*, %0** %3, align 8, !dbg !58
  %15 = load i8*, i8** %2, align 8, !dbg !59
  %16 = load i8*, i8** @OBJC_SELECTOR_REFERENCES_.2, align 8, !dbg !60, !invariant.load !12
  %17 = bitcast %0* %14 to i8*, !dbg !60
  %18 = call i8* bitcast (i8* (i8*, i8*, ...)* @objc_msgSend to i8* (i8*, i8*, i8*, i32)*)(i8* %17, i8* %16, i8* %15, i32 0), !dbg !60
  %19 = bitcast i8* %18 to %0*, !dbg !60
  call void @llvm.dbg.declare(metadata %0** %4, metadata !61, metadata !DIExpression()), !dbg !62
  %20 = load %struct._class_t*, %struct._class_t** @"OBJC_CLASSLIST_REFERENCES_$_", align 8, !dbg !63
  %21 = bitcast %struct._class_t* %20 to i8*, !dbg !63
  %22 = call i8* @objc_alloc(i8* %21), !dbg !63
  %23 = bitcast i8* %22 to %0*, !dbg !63
  %24 = load i8*, i8** %2, align 8, !dbg !64
  %25 = load i8*, i8** @OBJC_SELECTOR_REFERENCES_.2, align 8, !dbg !65, !invariant.load !12
  %26 = bitcast %0* %23 to i8*, !dbg !65
  %27 = call i8* bitcast (i8* (i8*, i8*, ...)* @objc_msgSend to i8* (i8*, i8*, i8*, i32)*)(i8* %26, i8* %25, i8* %24, i32 0), !dbg !65
  %28 = bitcast i8* %27 to %0*, !dbg !65
  store %0* %28, %0** %4, align 8, !dbg !62
  %29 = load %struct._class_t*, %struct._class_t** @"OBJC_CLASSLIST_REFERENCES_$_", align 8, !dbg !66
  %30 = bitcast %struct._class_t* %29 to i8*, !dbg !66
  %31 = call i8* @objc_alloc(i8* %30), !dbg !66
  %32 = bitcast i8* %31 to %0*, !dbg !66
  store %0* %32, %0** %4, align 8, !dbg !67
  %33 = load %0*, %0** %4, align 8, !dbg !68
  %34 = load i8*, i8** %2, align 8, !dbg !69
  %35 = load i8*, i8** @OBJC_SELECTOR_REFERENCES_.2, align 8, !dbg !70, !invariant.load !12
  %36 = bitcast %0* %33 to i8*, !dbg !70
  %37 = call i8* bitcast (i8* (i8*, i8*, ...)* @objc_msgSend to i8* (i8*, i8*, i8*, i32)*)(i8* %36, i8* %35, i8* %34, i32 0), !dbg !70
  %38 = bitcast i8* %37 to %0*, !dbg !70
  ret i32 0, !dbg !71
}

declare i8* @objc_alloc(i8*)

; Function Attrs: nonlazybind
declare i8* @objc_msgSend(i8*, i8*, ...) #3

attributes #0 = { noinline optnone ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { noinline optnone ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "darwin-stkchk-strong-link" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "probe-stack"="___chkstk_darwin" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nonlazybind }

!llvm.module.flags = !{!0, !1, !2, !3, !4, !5, !6, !7, !8, !9}
!llvm.dbg.cu = !{!10}
!llvm.ident = !{!24}

!0 = !{i32 2, !"SDK Version", [2 x i32] [i32 10, i32 15]}
!1 = !{i32 1, !"Objective-C Version", i32 2}
!2 = !{i32 1, !"Objective-C Image Info Version", i32 0}
!3 = !{i32 1, !"Objective-C Image Info Section", !"__DATA,__objc_imageinfo,regular,no_dead_strip"}
!4 = !{i32 4, !"Objective-C Garbage Collection", i32 0}
!5 = !{i32 1, !"Objective-C Class Properties", i32 64}
!6 = !{i32 2, !"Dwarf Version", i32 4}
!7 = !{i32 2, !"Debug Info Version", i32 3}
!8 = !{i32 1, !"wchar_size", i32 4}
!9 = !{i32 7, !"PIC Level", i32 2}
!10 = distinct !DICompileUnit(language: DW_LANG_ObjC, file: !11, producer: "Apple clang version 11.0.0 (clang-1100.0.33.16)", isOptimized: false, runtimeVersion: 2, emissionKind: FullDebug, enums: !12, retainedTypes: !13, nameTableKind: GNU)
!11 = !DIFile(filename: "main.m", directory: "/tmp/sc-WnCepcZTi")
!12 = !{}
!13 = !{!14}
!14 = !DICompositeType(tag: DW_TAG_structure_type, name: "FooClass", scope: !11, file: !11, line: 3, size: 64, flags: DIFlagObjcClassComplete, elements: !15, runtimeLang: DW_LANG_ObjC)
!15 = !{!16}
!16 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !14, baseType: !17, extraData: i32 0)
!17 = !DICompositeType(tag: DW_TAG_structure_type, name: "NSObject", scope: !11, file: !18, line: 53, size: 64, elements: !19, runtimeLang: DW_LANG_ObjC)
!18 = !DIFile(filename: "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include/objc/NSObject.h", directory: "")
!19 = !{!20}
!20 = !DIDerivedType(tag: DW_TAG_member, name: "isa", scope: !18, file: !18, line: 56, baseType: !21, size: 64, flags: DIFlagProtected)
!21 = !DIDerivedType(tag: DW_TAG_typedef, name: "Class", file: !11, line: 29, baseType: !22)
!22 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !23, size: 64)
!23 = !DICompositeType(tag: DW_TAG_structure_type, name: "objc_class", file: !11, flags: DIFlagFwdDecl)
!24 = !{!"Apple clang version 11.0.0 (clang-1100.0.33.16)"}
!25 = distinct !DISubprogram(name: "-[FooClass initWithBytes:length:]", scope: !11, file: !11, line: 11, type: !26, scopeLine: 11, flags: DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition, unit: !10, retainedNodes: !12)
!26 = !DISubroutineType(types: !27)
!27 = !{!28, !29, !30, !33, !35}
!28 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !14, size: 64)
!29 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !14, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!30 = !DIDerivedType(tag: DW_TAG_typedef, name: "SEL", file: !11, baseType: !31, flags: DIFlagArtificial)
!31 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !32, size: 64)
!32 = !DICompositeType(tag: DW_TAG_structure_type, name: "objc_selector", file: !11, flags: DIFlagFwdDecl)
!33 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !34, size: 64)
!34 = !DIDerivedType(tag: DW_TAG_const_type, baseType: null)
!35 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!36 = !DILocalVariable(name: "self", arg: 1, scope: !25, type: !28, flags: DIFlagArtificial | DIFlagObjectPointer)
!37 = !DILocation(line: 0, scope: !25)
!38 = !DILocalVariable(name: "_cmd", arg: 2, scope: !25, type: !39, flags: DIFlagArtificial)
!39 = !DIDerivedType(tag: DW_TAG_typedef, name: "SEL", file: !11, baseType: !31)
!40 = !DILocalVariable(name: "bytes", arg: 3, scope: !25, file: !11, line: 11, type: !33)
!41 = !DILocation(line: 11, column: 45, scope: !25)
!42 = !DILocalVariable(name: "l", arg: 4, scope: !25, file: !11, line: 11, type: !35)
!43 = !DILocation(line: 11, column: 63, scope: !25)
!44 = !DILocation(line: 12, column: 10, scope: !25)
!45 = !DILocation(line: 12, column: 8, scope: !25)
!46 = !DILocation(line: 14, column: 10, scope: !25)
!47 = !DILocation(line: 14, column: 3, scope: !25)
!48 = distinct !DISubprogram(name: "main", scope: !11, file: !11, line: 19, type: !49, scopeLine: 19, spFlags: DISPFlagDefinition, unit: !10, retainedNodes: !12)
!49 = !DISubroutineType(types: !50)
!50 = !{!35}
!51 = !DILocalVariable(name: "mem", scope: !48, file: !11, line: 20, type: !33)
!52 = !DILocation(line: 20, column: 15, scope: !48)
!53 = !DILocalVariable(name: "obj", scope: !48, file: !11, line: 21, type: !28)
!54 = !DILocation(line: 21, column: 13, scope: !48)
!55 = !DILocation(line: 21, column: 20, scope: !48)
!56 = !DILocation(line: 21, column: 51, scope: !48)
!57 = !DILocation(line: 21, column: 19, scope: !48)
!58 = !DILocation(line: 22, column: 4, scope: !48)
!59 = !DILocation(line: 22, column: 22, scope: !48)
!60 = !DILocation(line: 22, column: 3, scope: !48)
!61 = !DILocalVariable(name: "obj2", scope: !48, file: !11, line: 24, type: !28)
!62 = !DILocation(line: 24, column: 13, scope: !48)
!63 = !DILocation(line: 24, column: 21, scope: !48)
!64 = !DILocation(line: 24, column: 52, scope: !48)
!65 = !DILocation(line: 24, column: 20, scope: !48)
!66 = !DILocation(line: 25, column: 10, scope: !48)
!67 = !DILocation(line: 25, column: 8, scope: !48)
!68 = !DILocation(line: 26, column: 4, scope: !48)
!69 = !DILocation(line: 26, column: 23, scope: !48)
!70 = !DILocation(line: 26, column: 3, scope: !48)
!71 = !DILocation(line: 28, column: 3, scope: !48)
