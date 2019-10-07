; Generated from basic_c_support/phi.c and simplified by hand

declare i32 @foo()

define i32 @foobar() {
entry:
  br i1 true, label %cond.true, label %cond.false

cond.true:                                        ; preds = %entry
  %call = call i32 @foo()
  br label %cond.end

cond.false:                                       ; preds = %entry
  %call1 = call i32 @foo()
  br label %cond.end

cond.end:                                         ; preds = %cond.false, %cond.true
  %cond = phi i32 [ %call, %cond.true ], [ %call1, %cond.false ]
  ret i32 %cond
}

; After PHI node destruction 'foobar' should look like this:
;
; define i32 @foobar() {
; entry:
;   %cond.reg2mem = alloca i32
;   br i1 true, label %cond.true, label %cond.false
;
; cond.true:                                        ; preds = %entry
;   %call = call i32 @foo()
;   store i32 %call, i32* %cond.reg2mem
;   br label %cond.end
;
; cond.false:                                       ; preds = %entry
;   %call1 = call i32 @bar()
;   store i32 %call1, i32* %cond.reg2mem
;   br label %cond.end
;
; cond.end:                                         ; preds = %cond.false, %cond.true
;   %cond.reload = load i32, i32* %cond.reg2mem
;   ret i32 %cond.reload
; }