; ModuleID = 'main.m'
source_filename = "main.m"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.15.0"

%0 = type opaque

@OBJC_METH_VAR_NAME_ = private unnamed_addr constant [6 x i8] c"bytes\00", section "__TEXT,__objc_methname,cstring_literals", align 1
@OBJC_SELECTOR_REFERENCES_ = private externally_initialized global i8* getelementptr inbounds ([6 x i8], [6 x i8]* @OBJC_METH_VAR_NAME_, i32 0, i32 0), section "__DATA,__objc_selrefs,literal_pointers,no_dead_strip", align 8
@llvm.compiler.used = appending global [2 x i8*] [i8* getelementptr inbounds ([6 x i8], [6 x i8]* @OBJC_METH_VAR_NAME_, i32 0, i32 0), i8* bitcast (i8** @OBJC_SELECTOR_REFERENCES_ to i8*)], section "llvm.metadata"

; Function Attrs: noinline optnone ssp uwtable
define i8* @doStuff(%0*) #0 !dbg !14 {
  %2 = alloca %0*, align 8
  store %0* %0, %0** %2, align 8
  call void @llvm.dbg.declare(metadata %0** %2, metadata !36, metadata !DIExpression()), !dbg !37
  %3 = load %0*, %0** %2, align 8, !dbg !38
  %4 = load i8*, i8** @OBJC_SELECTOR_REFERENCES_, align 8, !dbg !39, !invariant.load !12
  %5 = bitcast %0* %3 to i8*, !dbg !39
  %6 = call i8* bitcast (i8* (i8*, i8*, ...)* @objc_msgSend to i8* (i8*, i8*)*)(i8* %5, i8* %4), !dbg !39
  ret i8* %6, !dbg !40
}

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nonlazybind
declare i8* @objc_msgSend(i8*, i8*, ...) #2

attributes #0 = { noinline optnone ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "darwin-stkchk-strong-link" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "probe-stack"="___chkstk_darwin" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { nonlazybind }

!llvm.module.flags = !{!0, !1, !2, !3, !4, !5, !6, !7, !8, !9}
!llvm.dbg.cu = !{!10}
!llvm.ident = !{!13}

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
!10 = distinct !DICompileUnit(language: DW_LANG_ObjC, file: !11, producer: "Apple clang version 11.0.0 (clang-1100.0.33.16)", isOptimized: false, runtimeVersion: 2, emissionKind: FullDebug, enums: !12, nameTableKind: GNU)
!11 = !DIFile(filename: "main.m", directory: "/tmp/sc-UCL84Tmbl")
!12 = !{}
!13 = !{!"Apple clang version 11.0.0 (clang-1100.0.33.16)"}
!14 = distinct !DISubprogram(name: "doStuff", scope: !11, file: !11, line: 3, type: !15, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !10, retainedNodes: !12)
!15 = !DISubroutineType(types: !16)
!16 = !{!17, !19}
!17 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !18, size: 64)
!18 = !DIDerivedType(tag: DW_TAG_const_type, baseType: null)
!19 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !20, size: 64)
!20 = !DICompositeType(tag: DW_TAG_structure_type, name: "NSData", scope: !11, file: !21, line: 70, size: 64, elements: !22, runtimeLang: DW_LANG_ObjC)
!21 = !DIFile(filename: "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/Frameworks/Foundation.framework/Headers/NSData.h", directory: "")
!22 = !{!23, !31, !35}
!23 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !20, baseType: !24, extraData: i32 0)
!24 = !DICompositeType(tag: DW_TAG_structure_type, name: "NSObject", scope: !11, file: !25, line: 53, size: 64, elements: !26, runtimeLang: DW_LANG_ObjC)
!25 = !DIFile(filename: "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include/objc/NSObject.h", directory: "")
!26 = !{!27}
!27 = !DIDerivedType(tag: DW_TAG_member, name: "isa", scope: !25, file: !25, line: 56, baseType: !28, size: 64, flags: DIFlagProtected)
!28 = !DIDerivedType(tag: DW_TAG_typedef, name: "Class", file: !11, line: 5, baseType: !29)
!29 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !30, size: 64)
!30 = !DICompositeType(tag: DW_TAG_structure_type, name: "objc_class", file: !11, flags: DIFlagFwdDecl)
!31 = !DIObjCProperty(name: "length", file: !21, line: 72, attributes: 257, type: !32)
!32 = !DIDerivedType(tag: DW_TAG_typedef, name: "NSUInteger", file: !33, line: 13, baseType: !34)
!33 = !DIFile(filename: "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include/objc/NSObjCRuntime.h", directory: "")
!34 = !DIBasicType(name: "long unsigned int", size: 64, encoding: DW_ATE_unsigned)
!35 = !DIObjCProperty(name: "bytes", file: !21, line: 78, attributes: 257, type: !17)
!36 = !DILocalVariable(name: "data", arg: 1, scope: !14, file: !11, line: 3, type: !19)
!37 = !DILocation(line: 3, column: 29, scope: !14)
!38 = !DILocation(line: 4, column: 11, scope: !14)
!39 = !DILocation(line: 4, column: 10, scope: !14)
!40 = !DILocation(line: 4, column: 3, scope: !14)
