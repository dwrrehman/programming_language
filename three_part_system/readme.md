# A Compiler for A Programming Language

This project is a compiler for a C-like programming language, which is aimed at providing low level control, while keeping the language expression-based. The compiler's frontend and backend are currently under development, and this document serves to document the syntax and semantics of the language in detail. 

### Language Syntax

The language has a syntax similar to early languages like B, however it tries to simplify some of the artificial complexity caused by infix notation, types, functions, as well as strings/arrays. These things are acheived in slightly different ways, which ends up simplifying the language overall.

There are only very few grammatically valid statements in the language. Given that `E` could be any arbitrary expression, `S` could be any arbitrary list of statements, `N` could be any arbitrary identifier, the following are valid forms:

#### Assignments

- `define N = E` : defining a new register variable in the global scope. 

- `set @E = E` : a store of an expression value to a computed L-value address.

- `set N = E` : an simple assignment to a register variable.

#### Control Flow

- `if E then S else S end` : if-else statement, conditionally execute either the first block if `E` is nonzero, else execute the second block. 

- `repeat S while E` : repeat-while statement, which executes the block `S` until `E` evaulates to 0.

- `do N` : An unconditional goto statement to the label `N`. 

- `at N` : Attributes the label `N` to refer to location of the next statement directly after this statement.

#### Language-Specific Constructs

- `del N` : Delete the register variable `N` from the symbol table, at this point. 

- `emit E` : Emit one byte of data to the executable, calculated from expression `E`.

- `eoi` : End of Input. parsing stops after this statement is encountered. Not required, but sometimes useful.

- `macro-name-here ...macro-arguments...` : Macro call. This will be explained in more detail in the Macro section of this document.



Additionally, all expressions are written in prefix notation, which simplifies both the programmers mental model, and the frontend's parser, albeit at the cost of mild unfamiliarity. All operators take two arguments as input, and output one value unless otherwise stated. The following operators are defined as valid in expressions in this language:

#### Arithmetic Operators

- `+` : addition

- `-` subtraction

- `*` multiply

- `/` division

- `%` remainder

#### Bitwise Logical Operators

- `&` bitwise-and

- `|` bitwise-or

- `~` bitwise-exclusive-or

- `^` bitwise shift up

- `!` bitwise shift down

#### Other Operators

- `@` unary load/store 32/64-bit word

- `<` set to 1 if less than, else set to 0

- `=` set to 1 if equal to, else set to 0


As a result of this prefix-operator-based design, this means that parenthesis are never used in this language for the purposes of expressions. Thus, multi-line, nest-able comments are denoted with the following syntax:

```
(This is a comment!
    it can span multiple lines!
	(and can even contain other 
      other balanced 
 parenthesis inside!))
```


To use the above operators in expressions, a few examples of expressions and their C equivalent are given below.

```
(The below assignment computes the equivalent of "x = x + 1" in C.)
set x = + x 1

(does the equivalent of "r = a * b + c * d" in C.)
set r = + * a b * c d

(does the equivalent of "x = a < b ? b - a : a - b" in C.)
if < a b then set x = - b a else set x = - a b end

(does the same as the above statement, but branchless)
set x = * - b a * < a b - 0 1 

```












old example of the syntax:

```
at my cool thingy
	define my x = 1
	if x < 0001 then
		repeat
			set @(p + my x) = 0
			set my x = my x + 1
		while my x < 0001
		do done with everything
	else
		set my x = 0
		do my cool thingy
	end

at done with everything
	set my x = 11111
	eoi
```


















































```
hello this is a test
for (nat i = 0; i < count; i++) {
	puts("hi lol");
}
```

we will be creating the syntax and specifications for the programming language now!

we will first start with making the `for` construct, which will begin using the following features:

- the first feature
- the second feature
- the third feature


and then we will do the next thing, which is the following


```
// this is a comment lol
function(bubbles, beans) {

	// the users code goes here!
}
```


and as you can see, its quite easy! 



(end of the document)


