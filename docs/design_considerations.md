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

### C

#### Argument rewriting

If we have debug info, we can extract the source-level number, names and types of arguments from a function. However, this will not always correspond to the bitcode level signature. Let's say
```C
typedef struct {float x; float y; float z;} PointT;

float mean(PointT point){
  return (point.x + point.y + point.z) / 3;
}
```
If we compile this with `-O3`, we get
```
define dso_local float @mean(<2 x float>, float) local_unnamed_addr #0 {
  %3 = extractelement <2 x float> %0, i32 0
  %4 = extractelement <2 x float> %0, i32 1
  %5 = fadd float %3, %4
  %6 = fadd float %5, %1
  %7 = fdiv float %6, 3.000000e+00
  ret float %7
}
```
If we instead compile with `-O0`, we get
```
%struct.PointT = type { float, float, float }

; Function Attrs: noinline nounwind optnone sspstrong uwtable
define dso_local float @mean(<2 x float>, float) #0 {
  %3 = alloca %struct.PointT, align 4
  %4 = alloca { <2 x float>, float }, align 4
  %5 = getelementptr inbounds { <2 x float>, float }, { <2 x float>, float }* %4, i32 0, i32 0
  store <2 x float> %0, <2 x float>* %5, align 4
  %6 = getelementptr inbounds { <2 x float>, float }, { <2 x float>, float }* %4, i32 0, i32 1
  store float %1, float* %6, align 4
  %7 = bitcast %struct.PointT* %3 to i8*
  %8 = bitcast { <2 x float>, float }* %4 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %7, i8* align 4 %8, i64 12, i1 false)
  %9 = getelementptr inbounds %struct.PointT, %struct.PointT* %3, i32 0, i32 0
  %10 = load float, float* %9, align 4
  %11 = getelementptr inbounds %struct.PointT, %struct.PointT* %3, i32 0, i32 1
  %12 = load float, float* %11, align 4
  %13 = fadd float %10, %12
  %14 = getelementptr inbounds %struct.PointT, %struct.PointT* %3, i32 0, i32 2
  %15 = load float, float* %14, align 4
  %16 = fadd float %13, %15
  %17 = fdiv float %16, 3.000000e+00
  ret float %17
}

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i1 immarg) #1
```

### Objective C

#### Message Passing

Objective C has a smalltalk-inspired message passing dynamic dispatch. The way this works is that we can send any message to any object, where a message consists of `name`, static types of the arguments, and the arguments themselves. The object has a pointer to its dynamic type; and the dynamic type can either respond to a message (consume it) or pass it up to its supertype. If a message reaches the top of the class hierarchy, then we get a runtime error.

Consider ```acc += [s intValue]```

In IR, this is a call to `@objc_msgSend`, that takes both a pointer to the object, a pointer to the message, and the arguments. In IR we get something like
```
@OBJC_METH_VAR_NAME_.1 = private unnamed_addr constant [9 x i8] c"intValue\00", section "__TEXT,__objc_methname,cstring_literals", align 1
...
@OBJC_SELECTOR_REFERENCES_.2 = private externally_initialized global i8* getelementptr inbounds ([9 x i8], [9 x i8]* @OBJC_METH_VAR_NAME_.1, i32 0, i32 0), section "__DATA,__objc_selrefs,literal_pointers,no_dead_strip", align 8
...
  %38 = load i8*, i8** @OBJC_SELECTOR_REFERENCES_.2, align 8, !dbg !77, !invariant.load !12
  ...
  %40 = call i32 bitcast (i8* (i8*, i8*, ...)* @objc_msgSend to i32 (i8*, i8*)*)(i8* %39, i8* %38), !dbg !77
```
In order to figure out what message gets sent (`intValue`) we need to track through all the indirections. We can probably do that in the frontend.

Note the very useful `!invariant.load` tag. It informs LLVM that the this pointer load is save to hoist out of loops (regardless whether the target could be written to). For us, it is a hint to (illegally!) treat `@OBJC_SELECTOR_REFERENCES_.2` as a constant (even though it is not; it is a mutable global variable that happens to be initialized to be a pointer to the right string).

AFAIU this pointer dance serves to correctly intern (dedup) the `intValue\00` string when a dynamic library is loaded at run time: The loader will then change the new lib's `@OBJC_SELECTOR_REFERENCES_.2` to point at the string in the loading libraries `TEXT` section. Hence, message strings can be compared by pointer identity instead of string-compare.

#### Type Hierarchy

Consider the following example:

```objectivec
@interface Foo : NSObject

+ (Foo *)classMethod;
- (void)instanceMethod;

@end
```

In this example, the class `Foo` inherits from the class `NSObject` declared elsewhere. There are two methods,
`classMethod` and `instanceMethod` which belong to the class and to the instance respectively. Here an example of
how they used. We pass a message `classMethod` to the class object, and then `instanceMethod` is passed to the
instance object.

```objectivec
Foo *instance = [Foo classMethod];
[instance instanceMethod];
```

The following code would lead to a runtime exception:

```objectivec
Foo *instance = [Foo instanceMethod];
[instance classMethod];
```
Class and instance methods are differentiated by the `+` (class) or `-` (instance) symbol.

Class and instance methods may have the same name:

```objectivec
@interface ValidClass : NSObject

+ (void)foobar;
- (void)foobar;

@end

```

But two class or instance method names cannot collide:

```objectivec
@interface InvalidClass : NSObject

/// Collision despite different signatures 
+ (id)classMethod:(int)parameter;
+ (void)classMethod:(float)parameter;

/// Collision despite different signatures
- (id)instanceMethod:(int)parameter;
- (void)instanceMethod:(float)parameter;

@end
``` 

The closest analogy to the ObjC class methods in languages like C++ and Java are the static methods.
Unlike Java and C++, ObjC class methods resolved dynamically via ObjC runtime.

To handle class and instance methods correctly we build two parallel type hierarchies for ObjC classes. 
Considering the first example:

```objectivec
@interface Foo : NSObject

+ (Foo *)classMethod;
- (void)instanceMethod;

@end
```

we emit two `typeDecl`s for the `Foo` class: `Foo` for the class and `Foo$` for so-called metaclass.
Class and instance methods defined under the `Foo` typeDecl, while the metaclass `Foo$` does not have any:

```scala
ocular> cpg.typeDecl.nameExact("Foo").method.name.p
+classMethod
-instanceMethod
ocular> cpg.typeDecl.nameExact("Foo$").method.name.p
<empty>
```

Method bindings, though, attached to the class and metaclass depending whether they are class methods or class methods:

```scala
ocular> cpg.typeDecl.nameExact("Foo").boundMethod.name.p
-instanceMethod
ocular> cpg.typeDecl.nameExact("Foo$").boundMethod.name.p
+classMethod
```

This way, we can use different types at the call site to differentiate between instance and class methods correctly.

### C++

#### Templates

From the LLVM / binary viewpoint, `foo<int>` and `foo<float>` bear no relation whatsoever. The fact that both were spawned from the same template has not syntactic nor semantic impact, nor does it impact generated assembly. We can recover some kind of source-like name by demangling the name used by LLVM (i.e.: The name defined in the CXX-ABI; it is intended for consumption by the dynamic linker, and not humans).

Nevertheless, policy writers and end users want to think about source-style names, and will want to write queries / policies that match both `foo<int>` and `foo<float>`. Hence, we need some kind of wildcard support.

We discussed the option of using regular expressions. The problem is that regular expressions are both too general and not general enough. They are too general and therefore slow in allowing arbitrary patterns (We must have a way of getting polylog-linear time in number of policies + methods, instead of policies x methods). They are not general enough, because C++ names are not a regular language and cannot be parsed with regex (cf https://stackoverflow.com/questions/1732348/regex-match-open-tags-except-xhtml-self-contained-tags/1732454#1732454)): We cannot count nested template depths. 

We think that it is probably feasible to match wildcards like `cxx"MyNameSpace::foo<%T>"` or `cxx"%NS::foo<%T>"`, where `%T` consumes one template parameter.

For the sake of the data-flow tracker and CPG, `foo<int>` and `foo<bar>` will remain entirely separate entities that bear no relation whatsoever.


#### Library inlining

Clang will tend to emit template / header-only library code into the binary. This means we may have a lot of STL code in each CPG. We could try to special-case STL, but then there is boost and then the next hip template lib. Realistically, we should probably just ship the code to the backend, which uses the fancy template wildcard matching to attach semantics to the included library functions.


Clang sometimes inlines some library code into user-code, even if inlining is disabled (some STL code, macros). If we have debug symbols, then we can detect this, but there is currently nothing we can do about it, except leaving the LINENUMBER field blank.


#### Dynamic Dispatch

C++ supports two kinds of dynamic dispatch: First, calling function pointers. There is nothing new here. Second, calls of non-static virtual member functions, i.e. the vtable. Consider:

```C++
int callfoo(Base *arg){
    return arg->foo();
}
```
Let's look at the output. With `-O3`
```
define dso_local i32 @_Z7callfooP4Base(%class.Base*) local_unnamed_addr #2 {
  %2 = bitcast %class.Base* %0 to i8 (%class.Base*)***
  %3 = load i8 (%class.Base*)**, i8 (%class.Base*)*** %2, align 8, !tbaa !4
  %4 = getelementptr inbounds i8 (%class.Base*)*, i8 (%class.Base*)** %3, i64 1
  %5 = load i8 (%class.Base*)*, i8 (%class.Base*)** %4, align 8
  %6 = tail call signext i8 %5(%class.Base* %0)
  %7 = sext i8 %6 to i32
  ret i32 %7
}
!4 = !{!5, !5, i64 0}
!5 = !{!"vtable pointer", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
```
and with `-O0`:
```
define dso_local i32 @_Z7callfooP4Base(%class.Base*) #1 {
  %2 = alloca %class.Base*, align 8
  store %class.Base* %0, %class.Base** %2, align 8
  %3 = load %class.Base*, %class.Base** %2, align 8
  %4 = bitcast %class.Base* %3 to i8 (%class.Base*)***
  %5 = load i8 (%class.Base*)**, i8 (%class.Base*)*** %4, align 8
  %6 = getelementptr inbounds i8 (%class.Base*)*, i8 (%class.Base*)** %5, i64 1
  %7 = load i8 (%class.Base*)*, i8 (%class.Base*)** %6, align 8
  %8 = call signext i8 %7(%class.Base* %3)
  %9 = sext i8 %8 to i32
  ret i32 %9
}
```
It is noteworthy that `-O0` lacks the very useful `tbaa`-tag that informs us about the position of vtables.

From these codes we need to figure out that `Base::foo` was called (i.e. what message/vtable-slot is used? We see that it is slot 1, but we don't see immediately what the source-level base class is, nor the name `foo`; llvm names tend to be rather unreliable). I.e. we need to figure out vtable contents, and relate the vtables themselves to both types and constructors.

It is maybe useful to model C++ dynamic dispatch like objectiveC message passing: The message is `Base::foo`. However, it is harder to figure out this message.


## CFG construction

As mentioned above, we construct a flat/wide CPG from an LLVM function.

Each instruction is converted into a tree consisting of `CPGProtoNode`s.
Such trees constitute a set of top-level nodes. The top-level nodes are connected to the `method block` via AST edges.
This way, the tree structure forms the AST representation of bitcode.

Building of the CFG is the two-phase process.

1. Build CFG within top-level node (i.e. for each instruction).
2. Build CFG across top-level nodes (i.e. connect all instructions).

To do this we add a property `entry` to the `CPGProtoNode`.
The property holds the `id` (or `key`) of the next node in the CFG. There are two cases:

1. If the node is a leaf (i.e. does not have children), then the `entry` equals to the node's `id`.
2. Otherwise, the `entry` equals to an `entry` to the node's first child.

#### Build CFG within a top-level node

Then CFG nodes are added as follows:

1. Each child connects with an entry to its next sibling.
2. The last child (the one without siblings) connects to its parent node `id`. 

Here is the code:

```
void CPGEmitter::resolveConnections(CPGProtoNode *parent, std::vector<CPGProtoNode *> children) {
  if (children.empty()) {
    parent->setEntry(parent->getID());
    return;
  }

  parent->setEntry(children.front()->getEntry());
  builder.connectCFG(children.back()->getID(), parent->getID());

  for (size_t i = 0; i < children.size() - 1; i++) {
    CPGProtoNode *current = children[i];
    CPGProtoNode *next = children[i + 1];
    builder.connectCFG(current->getID(), next->getEntry());
  }
}
```

#### Build CFG across top-level nodes

This is also two-phase process:

1. Connect top-level nodes within a basic block.

These are just consecutive nodes. Same algorithm as above applies: 

```
for (size_t i = 0; i < nodes.size() - 1; i++) {
  CPGProtoNode *current = nodes[i];
  CPGProtoNode *next = nodes[i + 1];
  builder.connectCFG(current->getID(), next->getEntry());
}
```

2. Connect top-level nodes via branches

Each basic block ends with a terminator instruction. We do not process them immediately, but collect
and process after all the top level nodes are constructed. Take a look at `CPGEmitter::emitMethod` 
for more details.


# Type Deduplication

## Problem Statement

Consider the following example:

```c
// Point.h
struct Point {
  int x;
  int y;
};
```

```c
// foo.c
#include "Point.h"

// use struct Point
```

```c
// bar.c
#include "Point.h"

// use struct Point
```

When `foo.c` and `bar.c` compiled down to the LLVM IR (`foo.ll` and `bar.ll`) they both have `struct Point` as follows:

```llvm
%struct.Point = type { i32, i32 }
```

Though, when both IR files loaded in one context the type names changed to prevent name collisions, so they end up being defined as

```llvm
%struct.Point = type { i32, i32 }
%struct.Point.0 = type { i32, i32 }
```

We want to deduplicate such types.

These are the options we considered:

1. `IRLinker`/`llvm-link` it works OK-ish, but far away from being good: it drops some important information. 

The following IR after running through `IRLinker`

```llvm
%struct.Point = type { i32, i32 }
%struct.Tuple = type { i32, i32 }
```

becomes

```llvm
%struct.Point = type { i32, i32 }
```

dropping the other struct since both have the same layout.
We don't want to lose this information.

2. `llvm::StructType::isLayoutIdentical` function is supposed to tell whether two are struct have the same layout.
Then, we can pick the IR

```llvm
%struct.Point = type { i32, i32 }
%struct.Point.0 = type { i32, i32 }
```

group structs by their canonical name (dropping `struct.` and anything that goes after the next dot),
and then check if they have the same layout. If that's the case, we consider both structs to be the same and only emit one
`typeDecl` for both.

However, this approach does not work for the following code:

```llvm
%struct.Point = type { i32, i32 }
%struct.Point.0 = type { i32, i32 }

%struct.Wrapper = type { %struct.Point }
%struct.Wrapper.0 = type { %struct.Point.0 }
```

According to LLVM, `Point` and `Point.0` have an identical layout, while `Wrapper` and `Wrapper.0` don't. That is because
LLVM compares types by pointer: `i32` is reused and points to the same `llvm::Type` instance, which is not the case for `Point` and `Point.0`.

## Current Solution

We are not aware of any other present solutions for the problem, so we roll out our own.

### A bit of background

Our implementation is inspired by [Tree Automata](https://en.wikipedia.org/wiki/Tree_automaton) and [Ranked Alphabet](https://en.wikipedia.org/wiki/Ranked_alphabet).

Here is a short description: a ranked alphabet consists of a finite set of symbols _F_, and a function _Arity(f), where f belongs to F_.
The _Arity_ tells how many arguments a symbol _f_ has. Symbols can be constant, unary, binary, ternary, or n-ary.

Here is an example of the notation: `a`, `b`, `f(,)`, `g()`, `h(,,,,)`, `a` and `b` are constants, `f(,)` is binary, `g()` is unary, and `h(,,,,)` is n-ary.
Arity for each symbol is 0, 0, 2, 1, and 5, respectively.

Given the alphabet `a`, `b`, `f(,)`, `g()` we can construct a number of trees:

 - f(a, b)
 - g(b)
 - g(f(b, b))
 - f(g(a), f(f(a, a), b))
 - f(g(a), g(f(a, a)))
 
etc.

If we know the arity of each symbol, then we can omit parentheses and commas and write the tree as a string.
The tree is constructed in the depth-first order, here are the same examples as above, but in the string notation:

 - fab
 - gb
 - gfbb
 - fgaffaab
 - fgagfaa
 
### Type Equality

We consider each type to be a symbol and its arity is the number of properties we want to compare. Then, we build a tree of the type and convert it to the string representation.
If two types have the same string representation, then they are equal.

Some examples:

 - `i32`, `i64`, `i156`: symbol `I`, arity is 1 since we only care about bitwidth (e.g., 32, 64, 156)
 - `float`: symbol `F`, arity is 0, all `float` types are the same
 - `[16 x i32]`: symbol `A`, arity is 2, we care only about the length of the array and its element type
 - `i8*`: symbol `P`, arity is 1, we care only about the pointee type
 - `{ i32, [16 x i8], i8* }`: symbol `S`, arity is number of elements + 2. We want to store the struct ID and number of its elements.
 
If we care about more or fewer values, then we can simply increase the arity for a given symbol.
More examples with the above example in mind:

 - `i32` -> `I(32)` -> `I32`
 - `i177` -> `I(177)` -> `I177`
 - `[16 x i8*]` -> `A(16, P(I(8)))` -> `A16PI8`
 - `{ i32, i8*, float }` -> `S(3, S0, I(32), P(I(8)), F)` -> `S300I32PI8F`
 
_Note:_ the values in `S` are the number of elements (3), struct ID (`S0`), and all its nested types defined recursively

### Structural Equality

Above we mentioned the `struct ID`. We need it to define the structural equality for recursive types.
Consider the following example: 

```llvm
%list = type { %list*, i32 }
%node = type { %node*, i32 }
%root = type { %node*, i32 } 
``` 
All of the above structs have the same layout: a pointer + an integer. But we do not consider them all to be equal.
By our definition of equality the following holds:

```
list == node
root != node
root != list
```

The reasoning is simple: the `list` and `node` has the same layout and the same structure (recursive), while `root` has another structure.
In order to take this into account and to make the equality hold we do not use the names of the structures, but before building the tree we assign them a symbolic name or IDs.
So both the `list` and `node` encoded as the following: `S(2, S0, P(S(2, S0, x, x), I(32))` where `x` is the placeholder for a recursive struct definition,
while the `root` is defined as follows `S(2, S0, P(S(2, S1, P(S(2, S1, x, x), I(32), I(32))), I(32))` please note the nestedness and `S0` and `S1` struct IDs.

Given these two encodings, the comparison above holds.

### Opaque Struct Equality

Comparing opaque structs is something that is not easy to define. We also use symbolic names whenever we see an opaque struct.
But different opaque structs get the same symbolic name as soon as the have the same canonical name, example:

```llvm
%struct.A = type opaque
%struct.A.0 = type opaque
%struct.B = type opaque

%foo = type { %struct.A* }
%bar = type { %struct.A.0* }
%buzz = type { %struct.B* }
``` 
Here, the canonical names for opaque structs are `A` (`%struct.A`, `%struct.A.0`) and `B` (`%struct.B`).
Therefore, we consider the `%struct.A` and `%struct.A.0` as equal, while `%struct.B` is not equal to either of `A`s.
Even though, all of the 3 structs can point to the same type, or to completely different types.  