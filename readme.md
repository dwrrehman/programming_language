# Untitled Programming Language

This is a compiler for a general-purpose systems programming language. it is expression based, and as low-level as raw asssembly.

### The language's priorities:

 - __Brutal simplicity and minimalism__: The compiler's _total_ source code is limited to 500 lines.

 - __Maximal runtime performance__: The language aims to be faster than C.

 - __Code readability__: There is literally no syntax in the language.

### The language's traits:

 - It is strictly imperative and procedural, and compiled.

 - It is strongly and statically typed, according to the hardware's types.

 - It provides immense control over the target assembly language.

 - Has typed, hygenic macros, for zero-overhead abstractions.

 - Has its own standard library, to allow for freestanding targets with no dependencies.

The implementation for this compiler is written in C, with no dependencies. This compiler will be used to bootstrap the real compiler once this one is finished.

Created By Daniel Warren Riaz Rehman.
