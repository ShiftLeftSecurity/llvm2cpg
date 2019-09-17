## References

Most important ref is the [langref](https://llvm.org/docs/LangRef.html). Unfortunately the langref is incomplete and suboptimal for processor-specific intrinsics (i.e. for people who use `immintrin.h` for more convenient assembly-style simd programming). This will probably cause us some headaches. Best approach for docs is to read clang's `immintrin.h`, the llvm unit tests, and the compilation output of clang. We should also look at LLVM unit tests as a potentially useful basis to ensure that we can handle the entirety of the language.

An extremely convenient web tool for testing small things is [godbolt](https://godbolt.org/) (set options `-emit-llvm` with `x86-64 clang 8.0.0`, and typically `-O0` or `-O1`).

Otherwise, `$ clang -S -O0 -c -emit-llvm foo.c` are usable options to get a human-readable `foo.ll`.

## Flat versus deep CPGs

Consider
```C
int foo(int);

void fun1(int a, int b){
    foo(a+b);
    return;
}

void fun2(int a, int b){
    int tmp = a+b;
    foo(tmp);
    return;
}

void fun3(int a, int b){
    int tmp = a+b;
    foo(tmp);
    foo(tmp);
    return;
}

void fun4(int a, int b){
    int tmp1 = a+b;
    foo(tmp1);
    int tmp2 = a+b;
    foo(tmp2);
    return;
}

void fun5(int a, int b){
    foo(a+b);
    foo(a+b);
    return;
}
```
From a naive point of view, `fun1` has a deeper AST than `fun2`. However, the compiler will more or less generate identical bitcode for both:
```llvm
define dso_local void @_Z4fun1ii(i32 %0, i32 %1) #0 !dbg !7 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  store i32 %0, i32* %3, align 4
  store i32 %1, i32* %4, align 4
  %5 = load i32, i32* %3, align 4, !dbg !16
  %6 = load i32, i32* %4, align 4, !dbg !17
  %7 = add nsw i32 %5, %6, !dbg !18
  %8 = call i32 @_Z3fooi(i32 %7), !dbg !19
  ret void, !dbg !20
}

define dso_local void @_Z4fun2ii(i32 %0, i32 %1) #0 !dbg !21 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  store i32 %0, i32* %3, align 4
  store i32 %1, i32* %4, align 4
  %6 = load i32, i32* %3, align 4, !dbg !28
  %7 = load i32, i32* %4, align 4, !dbg !29
  %8 = add nsw i32 %6, %7, !dbg !30
  store i32 %8, i32* %5, align 4, !dbg !27
  %9 = load i32, i32* %5, align 4, !dbg !31
  %10 = call i32 @_Z3fooi(i32 %9), !dbg !32
  ret void, !dbg !33
}
```
Recovering the "deep" form of the CPG is a nontrivial decompilation problem that is best shelved for future consideration (the CPG for `fun3` can only contain a single addition, instead of two additions; the second function call forces us to emit a temp variable). Simply counting number of uses is not enough, because of the ordering of side-effects.

The simplest approach is to simply emit temporary variables for both SSA values and for `alloca`d local variables. This will lead to a very flat and large CPG/AST, instead of a deep one. However, the same considerations apply in java, and the dataflow tracker can deal with it.

## PHI-nodes

The mighty PHI node allows values to depend on control flow. It is very useful in SSA-form languages to propagate values out of blocks: An SSA-value is alive and accessible only in blocks that are dominated by its definition. Consider

```C
int foo();
int bar();

int foobar(bool b){
    return b ? foo() : bar();
}
```
yielding
```llvm

; Function Attrs: noinline nounwind optnone sspstrong uwtable
define dso_local i32 @foobar(i32) #0 {
  %2 = alloca i32, align 4
  store i32 %0, i32* %2, align 4
  %3 = load i32, i32* %2, align 4
  %4 = icmp ne i32 %3, 0
  br i1 %4, label %5, label %7

; <label>:5:                                      ; preds = %1
  %6 = call i32 (...) @foo()
  br label %9

; <label>:7:                                      ; preds = %1
  %8 = call i32 (...) @bar()
  br label %9

; <label>:9:                                      ; preds = %7, %5
  %10 = phi i32 [ %6, %5 ], [ %8, %7 ]
  ret i32 %10
}
```
This example demonstrates that PHI-nodes are emitted even for `-O0`. It also neatly demonstrates how PHI-nodes work on a conceptual level: assign to `%11` the value `%7` (i.e. `foo()`) if we come from the basic block with label 5, and assign the value `%8` (i.e. `bar()`) if we come from the basic block with label `7`.

### PHI-node lowering

The CPG currently does not support PHI-nodes. This means that we need to do *something*. There are roughtly three possibilities:
(1) Implement PHI-node support in CPG and backend / dataflow tracker
(2) Implement a transformation on llvm code that removes phi-nodes (effectively a compiler pass)
(3) Deal with it during CPG generation.

These are always the three possibilities when looking at language constructs that don't have direct backend support. Currently we tend towards (2), possibly using the reg2mem pass or writing our own. We would remove the phi-nodes by making an alloca in the first shared dominator of all the branch sources, then writing to the alloca, and reading it at the phi position. Unfortunately, reg2mem does currently not do the job, but we might be able to fork it to suit our needs.

## Optimization Passes

Frontends tend to emit extremely verbose code with `-O0`, and rely on llvm to clear that up. This means that we definitely want to run some optimization passes, in order to prevent the dataflow engine from choking.

In theory, debug and metadata (e.g. provenance info) is propagated during optimization; in practice, this does not work perfectly. We want to stay close to the original source, and many optimizations create crazy code (e.g. loop-rotate, loop unroll, auto-vectorizer), while some are definitely nice for us (e.g. instcombine) and some are maybe questionable (e.g. LICM). 

Figuring out the right optimization passes will take some fiddling. We probably want customers to give us `-O0` output, and then run our own idiosyncratic collection and order of transformation passes. There is a potential issue with llvm version mismatches: That is, if the customer uses a slightly different llvm version than our intermediate optimization passes, there might be dragons with bitcode stability. On the other hand, the result of optimization passes is even more unstable (even in point-releases, these are all pretty heuristic), so we might be forced to have different llvm versions between customer and llvm2cpg. The optimal selection of passes will almost surely be language specific (e.g. different for C vs objective C vs C++ vs Rust vs Swift).

## Language specific handling

We will almost surely want to include at least some language specific handling of constructs like C++ vtables, objective C-style vtables, etc. Attempting "user gives us bitcode without more info" is probably a unrealistic if we want good analysis results. So the goal might rather be to have many llvm front-end languages like objectivec2cpg or c2cpg or cpp2cpg that all share a solid llvm2cpg, not have a single llvm2cpg that does everything on its own. Currently we just need to keep that in mind for our code, in order to be prepared to later include language specific parts.

