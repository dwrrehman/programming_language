# Untitled Programming Language

This is a compiler for a general-purpose systems programming language.

### The language's priorities:

 - __Brutal simplicity and minimalism__: The compiler's _total_ source code is limited to 1000 lines.

 - __Maximal runtime performance__: The language aims to be faster than _by-hand assembly_.

 - __Code readability__: There is literally no syntax in the language.

### The language's traits:

 - It is strictly imperative and procedural, and compiled.

 - It is strongly, statically, nominally, and dependently typed.

 - It heavily and exclusively relies on the LLVM compiler infrastructure.

The implementation for this compiler is written in C, and uses the LLVM C API. This compiler will be used to bootstrap the real compiler once this one is finished.

Created By Daniel Warren Riaz Rehman.
