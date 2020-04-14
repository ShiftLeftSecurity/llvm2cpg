## [0.6.0] - 14 Apr 2020

- Strict mode added (`-strict-mode`)
- Added warnings for missing debug information and for the missing 'recommended flags' 

## [0.5.0] - 30 Mar 2020

- C/ObjC string inlining
- Construct correct ObjC type hierarchy across several modules
- ObjC categories now emitted as well
- Methods and namespaces got the FILE property attached 

## [0.4.1] - 28 Feb 2020

- Lowers/removes exception handling that doesn't play well with Phi-node elimination
- Introduces new command line option "-simplify" that runs some simplifying transformation on the bitcode before generating the CPG

## [0.4.0] - 24 Feb 2020

- Type deduplication: equal types from separate modules merged into one
- Debug Info for struct/class members and arguments/local variables
- Correct semantics for the bitcasts
- Correct semantics for the getelemntptr

## [0.3.0] - 13 Jan 2020

- String literals now emitted as part of CPG
- Less indirect calls (function pointers casts promoted to the static calls)
- Inline assembly is treated as a static call
- Structs now have (unnamed) members

## [0.2.1] - 09 Dec 2019

- ObjC class method call resolution

## [0.2.0] - 06 Dec 2019

- Better (demangled) names for C++ methods
- Internal inliner: improves reachability resolution
- ObjC type hierarchy (classes + methods)
- Initial ObjC call trees resolution

## [0.1.0] - 06 Nov 2019

- Initial LLVM support: most of the instructions have the right semantics defined
- Initial debug info support: line/column numbers, most of the function and variable names are handled
- Initial data flow support: scalar variables tracked
