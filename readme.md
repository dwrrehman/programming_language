# Untitled Programming Language
Created By Daniel Warren Riaz Rehman.

This is a compiler for a general-purpose systems programming language. it is expression based, and as low-level as raw asssembly.

### The language's priorities:

 - __Brutal simplicity and minimalism__: The compiler's _total_ source code is limited to 500 lines.

 - __Maximal runtime performance__: The language aims to be faster than C.

 - __Code readability__: There is literally no syntax in the language.

### The language's traits:

 - It doesnt have any syntax, because it uses the Universal Call Syntax Resolution (UCSR) algorithm, which allows for user-defined syntax.

 - It is strictly imperative and procedural, and compiled.

 - It is strongly and statically typed, according to the _hardware's_ types.

 - It provides _full_ control over the target assembly language.

 - Has strongly typed, hygenic macros, for creating novel truly zero-overhead abstractions.

 - Has its own standard library with no dependencies, to allow for freestanding targets.

The implementation for this compiler is written in C, with no dependencies. This compiler will be used to bootstrap the real compiler once this one is finished.

### Compiler Implementation Limits:

 - The language can only provide a single parsing error message per file, because of how the UCSR algorithm works.

 - only up to 32,767 (2^15 - 1) signatures in a context.

 - only up to 32,767 expression calls per file.

 - a maximum expression depth of 32,767 for a file.

 - only up to 62 (64 - 2) bytes allowewd per signature. 

 - only up to 30 (32 - 2) arguments allowed per expression call.

In addition to encouraging the writing of smaller and simpler programs, these limits allow for a faster compiler implementation.

### Language Intrinsic System:

There are currently 12 intrinsics for the initial context.

 0. error\x00 :  signfies resolution error.

 1. name\x00 : the signature/name type parameter designator.

 2. \_\x01\x01 : i0 parameter designator (the unit type).

 3. a\x01\x01 : character literal 'a'. 

 4. b\x01\x01 : character literal 'b'. 

 5. c\x01\x01 :  character literal 'c'. 

 6. .\x01 : variable signature delimiter. 

 7. join\x02\x02\x02 : used to allow for muliple statements.

 8. nop\x02 : generates "no operation" instruction. 

 9. del\x01\x02 : change delimiter to another character.

 10. def\x01\x02 : intrinsic for define a macro signature. 

 11. attach\x00\x02 : intrinsc for attaching definition. gives a definition for a macro symbol only.


