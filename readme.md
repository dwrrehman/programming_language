# Untitled Programming Language

This is a compiler for a general-purpose systems programming language.

### The language's priorities:

 - __Brutal simplicity and minimalism__: compiler's _total_ source code is _limited to 1000 lines_.

 - __Maximal runtime performance__: aims to be faster than _by-hand assembly_.

 - __Code readability__: There is _literally no syntax_ in the language.

### The language's traits:

 - It is strictly _imperative and procedural_, and _compiled_.

 - It is strongly, statically, nominally, and _dependently_ typed.

 - It heavily and exclusively relies on the _LLVM compiler infrastructure_.

The implementation for this compiler is written in C, and uses the LLVM C API. This compiler will be used to bootstrap the real compiler once this one is finished.

Created By Daniel Warren Riaz Rehman.
