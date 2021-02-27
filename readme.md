# my programming language
###### Created By Daniel Warren Riaz Rehman.

This is a compiler for a general-purpose, expression-based, imperative, low-level minimalist systems programming language. 


### Language priorities:

The language has only _two_ priorities:

 - __Maximal runtime performance__: The language should allow a skilled programmer to write programs that:
	- execute _as fast as possible_ on the given target, and/or
	- have a memory size _as small as possible_.

 - __Brutal simplicity and minimalism__: The compiler's _total_ source code is limited to 500 lines.


### What this language does NOT care about:

 - Portability, at the expense of control.

 - Easy interoperability with existing piece of code.

 - Conforming to any ABI. 

 - Backwards-compatability.

 - Anything legacy.

 - Compiletime performance. 

 - Giving many error messages to the user.

 - Maintaining a "mathematical purity" which gets in the way of runtime performance.

 - All the "cool trendy features" in high-level programming languages these days. 

 - Providing any debug or type information at runtime, or any information extraneous to execution. 

 - How abstract or extensible / "future-proof" the code is. 

 - Any language-level abstraction which incurs even the slightest performance cost. 

 - Things in the language having to be familiar or easy to pick up for programmers.

 - Anything that is easy to understand, but is inefficient in runtime performance.
 
 - Anything even remotely related to OOP.

if you think a programming language should have any one of these things at the expense of maximally acheiving the two language priorities laid out earlier, you should probably stop reading this, go away, and look up some other more inferior programming language project, because this language is NOT for you.

### Other information:

 - The programming language uses the Universal Call Syntax Resolution algorithm, for the front end.

 - The compiler currently aims to target x86-32, x86-64, Arm32, Arm64, Wasm32, and Wasm64. 

 - The current compiler implementation is written in a subset of C, and will be self-hosted soon.

 - The output limit must be a non zero multiple of 4.

 - Only a single input file is allowed per object file, currently.

 - 


