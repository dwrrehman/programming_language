# Untitled Programming Language
Created By Daniel Warren Riaz Rehman.

This is a compiler for a general-purpose, expression-based, low-level systems programming language.

### The language's priorities:

 - __Brutal simplicity and minimalism__: The compiler's _total_ source code is limited to 500 lines.

 - __Maximal runtime performance__: The language aims to be _faster_ than C.

 - __Code readability__: The language has _no_ syntax.

### The language's traits:

 - Uses the Universal Call Syntax Resolution (UCSR) algorithm.

 - Is strictly imperative, and compiled.

 - Is strongly and statically typed, and only uses _hardware-level_ types.

 - Has ability to generate any instruction in the target assembly language.

 - Has strongly-typed hygenic macros, for creating used-defined zero-overhead abstractions.

 - Has no dependencies in the standard library.

 - Allows for freestanding targets.

 - This compiler is written in C, and has no dependencies.

### Compiler Implementation Limits:

The Following limits allow for a faster compiler implementation. They also encourage the writing of smaller and simpler programs.

 - maximum 1 parsing error message per file.

 - maximum 32,767 (2^15 - 1) signatures in a context.

 - maximum 32,767 expression calls per file.

 - maximum expression depth of 32,767 per file.

 - maximum 62 (64 - 2) bytes allowed per signature. 

 - maximum 30 (32 - 2) arguments allowed per expression call.

### Language Intrinsic System:

The Language currently utilizes the following signatures in its intrinsic system. 

Each signature is given as a string of bytes, where a character which has an ASCII value less than 33 is interpreted as a type. The last byte in the signature is always a type, and denotes the type of the signature. All other types are parameters of that type.

**Program Types:**

 ```name\x00``` : the signature/name type. (name parameter type)

 ```_\x01\x01``` : the unit type. (i0 parameter type)

**Signature Elements:**

 ```a\x01\x01``` : character literal 'a'. 

 ```b\x01\x01``` : character literal 'b'. 

 ```c\x01\x01``` :  character literal 'c'. 

 ```.\x01``` : variable signature delimiter. 

 ```del\x01\x02``` : change delimiter to another character.

**Target Assembly Instructions:**

 ```nop\x02``` : generates "no operation" assembly instruction. 

**Context Intrinsics:**

 ```def\x01\x02``` : intrinsic for define a macro signature. 

 ```attach\x00\x02``` : intrinsc for attaching definition. gives a definition for a macro symbol only.

**Other:**

 ```error\x00``` : used to denote when the program had a parsing error.

 ```join\x02\x02\x02``` : used to allow for muliple statements.
