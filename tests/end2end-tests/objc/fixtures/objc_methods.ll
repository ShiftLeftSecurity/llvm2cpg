; Generated from objc_methods.m

%0 = type opaque
%1 = type opaque
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
@"OBJC_CLASS_$_RootClass" = global %struct._class_t { %struct._class_t* @"OBJC_METACLASS_$_RootClass", %struct._class_t* null, %struct._objc_cache* @_objc_empty_cache, i8* (i8*, i8*)** null, %struct._class_ro_t* @"_OBJC_CLASS_RO_$_RootClass" }, section "__DATA, __objc_data", align 8
@"OBJC_METACLASS_$_RootClass" = global %struct._class_t { %struct._class_t* @"OBJC_METACLASS_$_RootClass", %struct._class_t* @"OBJC_CLASS_$_RootClass", %struct._objc_cache* @_objc_empty_cache, i8* (i8*, i8*)** null, %struct._class_ro_t* @"_OBJC_METACLASS_RO_$_RootClass" }, section "__DATA, __objc_data", align 8
@OBJC_CLASS_NAME_ = private unnamed_addr constant [10 x i8] c"RootClass\00", section "__TEXT,__objc_classname,cstring_literals", align 1
@OBJC_METH_VAR_NAME_ = private unnamed_addr constant [11 x i8] c"overridden\00", section "__TEXT,__objc_methname,cstring_literals", align 1
@OBJC_METH_VAR_TYPE_ = private unnamed_addr constant [8 x i8] c"v16@0:8\00", section "__TEXT,__objc_methtype,cstring_literals", align 1
@"_OBJC_$_CLASS_METHODS_RootClass" = internal global { i32, i32, [1 x %struct._objc_method] } { i32 24, i32 1, [1 x %struct._objc_method] [%struct._objc_method { i8* getelementptr inbounds ([11 x i8], [11 x i8]* @OBJC_METH_VAR_NAME_, i32 0, i32 0), i8* getelementptr inbounds ([8 x i8], [8 x i8]* @OBJC_METH_VAR_TYPE_, i32 0, i32 0), i8* bitcast (void (i8*, i8*)* @"\01+[RootClass overridden]" to i8*) }] }, section "__DATA, __objc_const", align 8
@"_OBJC_METACLASS_RO_$_RootClass" = internal global %struct._class_ro_t { i32 3, i32 40, i32 40, i8* null, i8* getelementptr inbounds ([10 x i8], [10 x i8]* @OBJC_CLASS_NAME_, i32 0, i32 0), %struct.__method_list_t* bitcast ({ i32, i32, [1 x %struct._objc_method] }* @"_OBJC_$_CLASS_METHODS_RootClass" to %struct.__method_list_t*), %struct._objc_protocol_list* null, %struct._ivar_list_t* null, i8* null, %struct._prop_list_t* null }, section "__DATA, __objc_const", align 8
@OBJC_METH_VAR_NAME_.1 = private unnamed_addr constant [10 x i8] c"inherited\00", section "__TEXT,__objc_methname,cstring_literals", align 1
@OBJC_METH_VAR_TYPE_.2 = private unnamed_addr constant [8 x i8] c"@16@0:8\00", section "__TEXT,__objc_methtype,cstring_literals", align 1
@"_OBJC_$_INSTANCE_METHODS_RootClass" = internal global { i32, i32, [2 x %struct._objc_method] } { i32 24, i32 2, [2 x %struct._objc_method] [%struct._objc_method { i8* getelementptr inbounds ([10 x i8], [10 x i8]* @OBJC_METH_VAR_NAME_.1, i32 0, i32 0), i8* getelementptr inbounds ([8 x i8], [8 x i8]* @OBJC_METH_VAR_TYPE_.2, i32 0, i32 0), i8* bitcast (i8* (%0*, i8*)* @"\01-[RootClass inherited]" to i8*) }, %struct._objc_method { i8* getelementptr inbounds ([11 x i8], [11 x i8]* @OBJC_METH_VAR_NAME_, i32 0, i32 0), i8* getelementptr inbounds ([8 x i8], [8 x i8]* @OBJC_METH_VAR_TYPE_, i32 0, i32 0), i8* bitcast (void (%0*, i8*)* @"\01-[RootClass overridden]" to i8*) }] }, section "__DATA, __objc_const", align 8
@"_OBJC_CLASS_RO_$_RootClass" = internal global %struct._class_ro_t { i32 2, i32 0, i32 0, i8* null, i8* getelementptr inbounds ([10 x i8], [10 x i8]* @OBJC_CLASS_NAME_, i32 0, i32 0), %struct.__method_list_t* bitcast ({ i32, i32, [2 x %struct._objc_method] }* @"_OBJC_$_INSTANCE_METHODS_RootClass" to %struct.__method_list_t*), %struct._objc_protocol_list* null, %struct._ivar_list_t* null, i8* null, %struct._prop_list_t* null }, section "__DATA, __objc_const", align 8
@OBJC_METH_VAR_NAME_.3 = private unnamed_addr constant [5 x i8] c"init\00", section "__TEXT,__objc_methname,cstring_literals", align 1
@OBJC_SELECTOR_REFERENCES_ = internal externally_initialized global i8* getelementptr inbounds ([5 x i8], [5 x i8]* @OBJC_METH_VAR_NAME_.3, i32 0, i32 0), section "__DATA,__objc_selrefs,literal_pointers,no_dead_strip", align 8
@OBJC_CLASS_NAME_.4 = private unnamed_addr constant [6 x i8] c"Child\00", section "__TEXT,__objc_classname,cstring_literals", align 1
@OBJC_METH_VAR_NAME_.5 = private unnamed_addr constant [9 x i8] c"newChild\00", section "__TEXT,__objc_methname,cstring_literals", align 1
@"_OBJC_$_CLASS_METHODS_Child" = internal global { i32, i32, [2 x %struct._objc_method] } { i32 24, i32 2, [2 x %struct._objc_method] [%struct._objc_method { i8* getelementptr inbounds ([9 x i8], [9 x i8]* @OBJC_METH_VAR_NAME_.5, i32 0, i32 0), i8* getelementptr inbounds ([8 x i8], [8 x i8]* @OBJC_METH_VAR_TYPE_.2, i32 0, i32 0), i8* bitcast (i8* (i8*, i8*)* @"\01+[Child newChild]" to i8*) }, %struct._objc_method { i8* getelementptr inbounds ([11 x i8], [11 x i8]* @OBJC_METH_VAR_NAME_, i32 0, i32 0), i8* getelementptr inbounds ([8 x i8], [8 x i8]* @OBJC_METH_VAR_TYPE_, i32 0, i32 0), i8* bitcast (void (i8*, i8*)* @"\01+[Child overridden]" to i8*) }] }, section "__DATA, __objc_const", align 8
@"_OBJC_METACLASS_RO_$_Child" = internal global %struct._class_ro_t { i32 1, i32 40, i32 40, i8* null, i8* getelementptr inbounds ([6 x i8], [6 x i8]* @OBJC_CLASS_NAME_.4, i32 0, i32 0), %struct.__method_list_t* bitcast ({ i32, i32, [2 x %struct._objc_method] }* @"_OBJC_$_CLASS_METHODS_Child" to %struct.__method_list_t*), %struct._objc_protocol_list* null, %struct._ivar_list_t* null, i8* null, %struct._prop_list_t* null }, section "__DATA, __objc_const", align 8
@"OBJC_METACLASS_$_Child" = global %struct._class_t { %struct._class_t* @"OBJC_METACLASS_$_RootClass", %struct._class_t* @"OBJC_METACLASS_$_RootClass", %struct._objc_cache* @_objc_empty_cache, i8* (i8*, i8*)** null, %struct._class_ro_t* @"_OBJC_METACLASS_RO_$_Child" }, section "__DATA, __objc_data", align 8
@OBJC_METH_VAR_NAME_.6 = private unnamed_addr constant [12 x i8] c"doSomething\00", section "__TEXT,__objc_methname,cstring_literals", align 1
@"_OBJC_$_INSTANCE_METHODS_Child" = internal global { i32, i32, [2 x %struct._objc_method] } { i32 24, i32 2, [2 x %struct._objc_method] [%struct._objc_method { i8* getelementptr inbounds ([12 x i8], [12 x i8]* @OBJC_METH_VAR_NAME_.6, i32 0, i32 0), i8* getelementptr inbounds ([8 x i8], [8 x i8]* @OBJC_METH_VAR_TYPE_, i32 0, i32 0), i8* bitcast (void (%1*, i8*)* @"\01-[Child doSomething]" to i8*) }, %struct._objc_method { i8* getelementptr inbounds ([11 x i8], [11 x i8]* @OBJC_METH_VAR_NAME_, i32 0, i32 0), i8* getelementptr inbounds ([8 x i8], [8 x i8]* @OBJC_METH_VAR_TYPE_, i32 0, i32 0), i8* bitcast (void (%1*, i8*)* @"\01-[Child overridden]" to i8*) }] }, section "__DATA, __objc_const", align 8
@"_OBJC_CLASS_RO_$_Child" = internal global %struct._class_ro_t { i32 0, i32 0, i32 0, i8* null, i8* getelementptr inbounds ([6 x i8], [6 x i8]* @OBJC_CLASS_NAME_.4, i32 0, i32 0), %struct.__method_list_t* bitcast ({ i32, i32, [2 x %struct._objc_method] }* @"_OBJC_$_INSTANCE_METHODS_Child" to %struct.__method_list_t*), %struct._objc_protocol_list* null, %struct._ivar_list_t* null, i8* null, %struct._prop_list_t* null }, section "__DATA, __objc_const", align 8
@"OBJC_CLASS_$_Child" = global %struct._class_t { %struct._class_t* @"OBJC_METACLASS_$_Child", %struct._class_t* @"OBJC_CLASS_$_RootClass", %struct._objc_cache* @_objc_empty_cache, i8* (i8*, i8*)** null, %struct._class_ro_t* @"_OBJC_CLASS_RO_$_Child" }, section "__DATA, __objc_data", align 8
@"OBJC_CLASSLIST_REFERENCES_$_" = internal global %struct._class_t* @"OBJC_CLASS_$_Child", section "__DATA,__objc_classrefs,regular,no_dead_strip", align 8
@OBJC_SELECTOR_REFERENCES_.7 = internal externally_initialized global i8* getelementptr inbounds ([9 x i8], [9 x i8]* @OBJC_METH_VAR_NAME_.5, i32 0, i32 0), section "__DATA,__objc_selrefs,literal_pointers,no_dead_strip", align 8
@OBJC_SELECTOR_REFERENCES_.8 = internal externally_initialized global i8* getelementptr inbounds ([12 x i8], [12 x i8]* @OBJC_METH_VAR_NAME_.6, i32 0, i32 0), section "__DATA,__objc_selrefs,literal_pointers,no_dead_strip", align 8
@"OBJC_LABEL_CLASS_$" = internal global [2 x i8*] [i8* bitcast (%struct._class_t* @"OBJC_CLASS_$_RootClass" to i8*), i8* bitcast (%struct._class_t* @"OBJC_CLASS_$_Child" to i8*)], section "__DATA,__objc_classlist,regular,no_dead_strip", align 8
@llvm.compiler.used = appending global [18 x i8*] [i8* getelementptr inbounds ([10 x i8], [10 x i8]* @OBJC_CLASS_NAME_, i32 0, i32 0), i8* getelementptr inbounds ([11 x i8], [11 x i8]* @OBJC_METH_VAR_NAME_, i32 0, i32 0), i8* getelementptr inbounds ([8 x i8], [8 x i8]* @OBJC_METH_VAR_TYPE_, i32 0, i32 0), i8* bitcast ({ i32, i32, [1 x %struct._objc_method] }* @"_OBJC_$_CLASS_METHODS_RootClass" to i8*), i8* getelementptr inbounds ([10 x i8], [10 x i8]* @OBJC_METH_VAR_NAME_.1, i32 0, i32 0), i8* getelementptr inbounds ([8 x i8], [8 x i8]* @OBJC_METH_VAR_TYPE_.2, i32 0, i32 0), i8* bitcast ({ i32, i32, [2 x %struct._objc_method] }* @"_OBJC_$_INSTANCE_METHODS_RootClass" to i8*), i8* getelementptr inbounds ([5 x i8], [5 x i8]* @OBJC_METH_VAR_NAME_.3, i32 0, i32 0), i8* bitcast (i8** @OBJC_SELECTOR_REFERENCES_ to i8*), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @OBJC_CLASS_NAME_.4, i32 0, i32 0), i8* getelementptr inbounds ([9 x i8], [9 x i8]* @OBJC_METH_VAR_NAME_.5, i32 0, i32 0), i8* bitcast ({ i32, i32, [2 x %struct._objc_method] }* @"_OBJC_$_CLASS_METHODS_Child" to i8*), i8* getelementptr inbounds ([12 x i8], [12 x i8]* @OBJC_METH_VAR_NAME_.6, i32 0, i32 0), i8* bitcast ({ i32, i32, [2 x %struct._objc_method] }* @"_OBJC_$_INSTANCE_METHODS_Child" to i8*), i8* bitcast (%struct._class_t** @"OBJC_CLASSLIST_REFERENCES_$_" to i8*), i8* bitcast (i8** @OBJC_SELECTOR_REFERENCES_.7 to i8*), i8* bitcast (i8** @OBJC_SELECTOR_REFERENCES_.8 to i8*), i8* bitcast ([2 x i8*]* @"OBJC_LABEL_CLASS_$" to i8*)], section "llvm.metadata"

define internal i8* @"\01-[RootClass inherited]"(%0* %self, i8* %_cmd) {
entry:
  %self.addr = alloca %0*, align 8
  %_cmd.addr = alloca i8*, align 8
  store %0* %self, %0** %self.addr, align 8
  store i8* %_cmd, i8** %_cmd.addr, align 8
  %0 = load %0*, %0** %self.addr, align 8
  %1 = bitcast %0* %0 to i8*
  ret i8* %1
}

define internal void @"\01+[RootClass overridden]"(i8* %self, i8* %_cmd) {
entry:
  %self.addr = alloca i8*, align 8
  %_cmd.addr = alloca i8*, align 8
  store i8* %self, i8** %self.addr, align 8
  store i8* %_cmd, i8** %_cmd.addr, align 8
  ret void
}

define internal void @"\01-[RootClass overridden]"(%0* %self, i8* %_cmd) {
entry:
  %self.addr = alloca %0*, align 8
  %_cmd.addr = alloca i8*, align 8
  store %0* %self, %0** %self.addr, align 8
  store i8* %_cmd, i8** %_cmd.addr, align 8
  ret void
}

define internal i8* @"\01+[Child newChild]"(i8* %self, i8* %_cmd) {
entry:
  %self.addr = alloca i8*, align 8
  %_cmd.addr = alloca i8*, align 8
  store i8* %self, i8** %self.addr, align 8
  store i8* %_cmd, i8** %_cmd.addr, align 8
  %0 = load i8*, i8** %self.addr, align 8
  %1 = call i8* @objc_alloc(i8* %0)
  %2 = bitcast i8* %1 to %1*
  %3 = load i8*, i8** @OBJC_SELECTOR_REFERENCES_, align 8
  %4 = bitcast %1* %2 to i8*
  %call = call i8* bitcast (i8* (i8*, i8*, ...)* @objc_msgSend to i8* (i8*, i8*)*)(i8* %4, i8* %3)
  ret i8* %call
}

declare i8* @objc_alloc(i8*)

declare i8* @objc_msgSend(i8*, i8*, ...)

define internal void @"\01-[Child doSomething]"(%1* %self, i8* %_cmd) {
entry:
  %self.addr = alloca %1*, align 8
  %_cmd.addr = alloca i8*, align 8
  store %1* %self, %1** %self.addr, align 8
  store i8* %_cmd, i8** %_cmd.addr, align 8
  ret void
}

define internal void @"\01+[Child overridden]"(i8* %self, i8* %_cmd) {
entry:
  %self.addr = alloca i8*, align 8
  %_cmd.addr = alloca i8*, align 8
  store i8* %self, i8** %self.addr, align 8
  store i8* %_cmd, i8** %_cmd.addr, align 8
  ret void
}

define internal void @"\01-[Child overridden]"(%1* %self, i8* %_cmd) {
entry:
  %self.addr = alloca %1*, align 8
  %_cmd.addr = alloca i8*, align 8
  store %1* %self, %1** %self.addr, align 8
  store i8* %_cmd, i8** %_cmd.addr, align 8
  ret void
}

define %1* @useChild() {
entry:
  %c = alloca %1*, align 8
  %0 = load %struct._class_t*, %struct._class_t** @"OBJC_CLASSLIST_REFERENCES_$_", align 8
  %1 = load i8*, i8** @OBJC_SELECTOR_REFERENCES_.7, align 8
  %2 = bitcast %struct._class_t* %0 to i8*
  %call = call i8* bitcast (i8* (i8*, i8*, ...)* @objc_msgSend to i8* (i8*, i8*)*)(i8* %2, i8* %1)
  %3 = bitcast i8* %call to %1*
  store %1* %3, %1** %c, align 8
  %4 = load %1*, %1** %c, align 8
  %5 = load i8*, i8** @OBJC_SELECTOR_REFERENCES_.8, align 8
  %6 = bitcast %1* %4 to i8*
  call void bitcast (i8* (i8*, i8*, ...)* @objc_msgSend to void (i8*, i8*)*)(i8* %6, i8* %5)
  %7 = load %1*, %1** %c, align 8
  ret %1* %7
}