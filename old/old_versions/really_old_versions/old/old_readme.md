# my programming language
###### created by Daniel Warren Riaz Rehman.

this is an interpreter for a general purpose, statement based, imperative, low level minimalist systems programming language. it is being used to make the programming language self hosted, 



# todos:

- add the full language isa. [very important]
- add constant's support. (ie, numbers, in various bases!) [pretty important]


- add mulitple file support, correctly. [meh]


x - add macros. [very important]














# old:

this is a compiler for a general-purpose, statement-based, imperative, low-level minimalist systems programming language. 


### Language priorities:

the language has only _two_ priorities:

 - __maximal control over runtime performance__: the language should allow a skilled programmer to write programs that:
	- execute _as fast as possible_ on the given target, and/or
	- have a memory size _as small as possible_.

 - __brutal simplicity and minimalism__: the compiler's _total_ source code is limited to 500 lines.


### What this language does NOT care about:

 - expression based syntax, or any positional syntax.

 - portability, at the expense of control.

 - easy interoperability with existing pieces of code.

 - conforming to any ABI. 

 - backwards-compatability.

 - giving many helpful error messages to the user.

 - maintaining a "mathematical purity" which gets in the way of runtime performance.

 - anything to do with functional programming, or lambda calculus.

 - all the "cool features" in high-level programming languages these days. 

 - providing any debug or type information at runtime, or any information extraneous to execution. 

 - how abstract or extensible / "future-proof" the code is. 

 - any language-level abstraction which incurs even the slightest performance cost. 

 - things in the language having to be familiar or easy to pick up for programmers.

 - anything that is easy to understand, but is inefficient in runtime performance.
 
 - anything even remotely related to OOP.

if you think a programming language should have any one of these things at the expense of maximally acheiving the two language priorities laid out earlier, you should probably stop reading this, go away, and look up some other more inferior programming language project, because this language is NOT for you.

### Other information:

 - The compiler currently aims to target RiscV64, RiscV32, Arm64, and Arm32 only.

 - The current compiler implementation is written in C, and will be self-hosted eventually.


