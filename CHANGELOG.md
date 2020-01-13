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