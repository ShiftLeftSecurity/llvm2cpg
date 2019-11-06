# llvm2cpg

This program creates code property graphs from LLVM bitcode.

## Intro

`llvm2cpg` takes LLVM Bitcode as an input.
Let's get a 'hello-world' running first, and then look at how to get Bitcode for a real project.

Create an LLVM IR file `main.ll` with the following contents:

```
; main.ll
declare void @callee(i32 %sink)
define void @caller(i32 %x) {
    call void @callee(i32 %x)
  ret void
}
```

And run `llvm2cpg` against it:

```
llvm2cpg -output-dir=`pwd` main.ll
```

You should see similar output:

```
[2019-11-06 10:30:05.543] [llvm2cpg] [info] More details: /var/folders/pp/lt3pgm5971n1qw7pp2g_bmfr0000gn/T/llvm2cpg-d36a81.log
[2019-11-06 10:30:05.543] [llvm2cpg] [info] Loading main.ll
[2019-11-06 10:30:05.544] [llvm2cpg] [info] Emitting CPG 1/1
[2019-11-06 10:30:05.544] [llvm2cpg] [info] Serializing CPG
[2019-11-06 10:30:05.545] [llvm2cpg] [info] Saving CPG on disk
[2019-11-06 10:30:05.546] [llvm2cpg] [info] CPG is successfully saved on disk: /tmp/sc-w4QviQTLdY/cpg.bin.zip
[2019-11-06 10:30:05.546] [llvm2cpg] [info] Shutting down
```

Now we can run `ocular`

```
./ocular.sh
```

and run a simple query against the CPG

```
ocular> workspace.reset
ocular> loadCpg("/tmp/sc-w4QviQTLdY/cpg.bin.zip")
ocular> val sink = cpg.method.name("callee").parameter
ocular> val source = cpg.method.name("caller").parameter
ocular> sink.reachableBy(source).allFlows.p
```

You should see the following output after the last command:

```
2019-11-06 10:34:14.344 [main] INFO mainTasksSize: 1, reachedEndNode: 1,
res18: List[String] = List(
  """ ______________________________________
 | tracked| lineNumber| method| file   |
 |=====================================|
 | x      | N/A       | caller| main.ll|
 | x      | N/A       | caller| main.ll|
 | sink   | N/A       | callee| main.ll|
"""
)
```

If that's the case, then read further and see how to get CPG for a real project.

## Getting Bitcode

LLVM Bitcode may take one of the following forms:

 - LLVM IR (human-readable representation)
 - LLVM Bitcode (bitstream representation)
 - Embedded Bitcode (bitstream representation embedded into a binary)

There are several ways to get LLVM Bitcode out of high-level source code.
This section describes these approaches, covering both basic mechanics and the real-world use cases.
It concludes with a list of known issues.

### 'Hello-world' version

Let's use the following program as an example:

```
/// main.c
extern int printf(const char *, ...);
void callee(int x) {
  printf("%d\n", x);
}
int main(int argc, char **argv) {
    callee(14);
    callee(42);
    return 0;
}
```

##### LLVM IR

To emit LLVM IR for the single file, one can use the following command:

```
clang -c -S -emit-llvm main.c -o main.ll
```

Emitted `main.ll` can be passed to the `llvm2cpg`:

```
llvm2cpg main.ll
```

##### LLVM Bitcode

There are two ways to get the bitstream representation.

```
clang -c -emit-llvm main.c -o main.bc
```

Or via LTO trick:

```
clang -c -flto main.c -o main.o
```

In these cases, both `main.o` and `main.bc` contain LLVM Bitcode:

```
> file main.o main.bc
main.o:  LLVM bitcode, wrapper x86_64
main.bc: LLVM bitcode, wrapper x86_64
```

Either of them can be passed to `llvm2cpg`.

##### Embedded Bitcode

This is the ideal case since it gives the most straightforward integration and can be easily added to an existing build system without affecting the resulting software.

```
> clang -fembed-bitcode main.c -o main
> ./main
14
42
```

The resulting `main` can be passed to `llvm2cpg` as is:

```
> llvm2cpg main
[2019-11-06 10:00:59.021] [llvm2cpg] [info] Loading main
[2019-11-06 10:00:59.027] [llvm2cpg] [info] Emitting CPG 1/1
[2019-11-06 10:00:59.028] [llvm2cpg] [info] Serializing CPG
[2019-11-06 10:00:59.028] [llvm2cpg] [info] Saving CPG on disk
[2019-11-06 10:00:59.029] [llvm2cpg] [info] CPG is successfully saved on disk: ./cpg.bin.zip
[2019-11-06 10:00:59.029] [llvm2cpg] [info] Shutting down
```

## Real-world version

Getting Bitcode for the real-world projects with all the different build systems is less straightforward, but still doable. One need to inject one of the following flags into the build system:

 - `-emit-llvm`
 - `-flto`
 - `-fembed-bitcode`

_Note: Alternatively, one can use [whole-program-llvm](https://github.com/travitch/whole-program-llvm)._

In the case of `-emit-llvm`, the build doesn't finish properly (linking fails since there are no object files produced), but all the bitcode files will be available.
In the case of `-flto`, the build succeeds, and all the intermediate object files, in fact, will contain bitcode.
In the case of `-fembed-bitcode`, the build succeeds, and the resulting binary contains required bitcode.

##### CMake

```
cmake -DCMAKE_C_FLAGS=-fembed-bitcode -DCMAKE_CXX_FLAGS=-fembed-bitcode source-root
```

##### Xcode

Add a flag to both `Other C Flags` and `Other Linker Flags`.

##### xcodebuild

```
xcodebuild OTHER_CFLAGS=-fembed-bitcode OTHER_CPLUSPLUSFLAGS=-fembed-bitcode OTHER_LDFLAGS=-fembed-bitcode
```

##### Other build systems

Consider looking into [whole-program-llvm](https://github.com/travitch/whole-program-llvm).


### Known issues

- `-fembed-bitcode` may not work on macOS if a project links a static library that was not compiled with embedded bitcode support
- if `-fembed-bitcode` is combined with `-flto`, then bitcode won't be embedded into a binary
- in some cases, `llvm2cpg` cannot read debug information emitted by Xcode's clang. In this case, everything still works, but the debug info is not taken into account.

## Getting CPG out of a project

Once you get the bitcode, the CPG emission is trivial. Here are typical commands you may want to run depending on the way you get bitcode.

`-emit-llvm`:
```
cd build-directory
llvm2cpg `find ./ -name "*.bc"`
# or
llvm2cpg `find ./ -name "*.ll"`
```

`-flto`:
```
cd build-directory
llvm2cpg `find ./ -name "*.o"`
```

`-fembed-bitcode`:
```
cd build-directory
llvm2cpg program-binary
```

`whole-program-llvm`:
```
cd build-directory
llvm2cpg bitcode.bc
```
