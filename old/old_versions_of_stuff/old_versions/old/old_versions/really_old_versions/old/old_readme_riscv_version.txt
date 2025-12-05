      a RISC-V cross assembler:   
 written on 202405127.013227 by dwrr. 
--------------------------------------

this is the assembler/compiler/interpreter for a RISC-V-like language that is designed to target mulitple ISA's, including RV32/64, arm32/64, and x86/x64. the language is similar to forth, as arguments are specified in postfix notation, via an argument stack. the language has a compiletime system that allows any runtime instruction to be executed at compiletime, excluding system calls. the language is somewhat terse in comparison to assembly, as most built in operators/instructions in the language are a single character. 




