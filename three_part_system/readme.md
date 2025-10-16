# A Compiler for A Programming Language

This project is a compiler for a C-like programming language, which is aimed at providing low level control, while keeping the language expression-based. The compiler's frontend and backend are currently under development, and this document serves to document the syntax and semantics of the language in detail. 

### Language Syntax

The language has a syntax similar to early languages like B, however it tries to simplify some of the artificial complexity caused by infix notation, types, functions, as well as strings/arrays. These things are acheived in slightly different ways, which ends up simplifying the language overall.

Also, notice there are no statement seperators required between vaild instructions. Due to the fully prefix nature of the grammar, these seperators are not neccessary for the parser to get the right parse. additionally, all whitespace which isnt neccessary for delimiting neighboring words are ignored. There are no strings, and only decimal literals are supported currently. 

There are user-defined variables in this language, defined using the "define" statement, and undefined using the "del" statement. Additionally, low-level control flow is provided using if statements, and goto statements, however more common loops can be made using the repeat-while statement. 

An interesting note about the semantics of variables in this language is that all variables are guaranteed to be stored in registers, and function as both pointers, and as well as unsigned integers of a particular bit-width. there is no type system currently, as there is only one type- the register word, which is stored using the natural number of bits for the given target being compiled for. On a 64-bit system, variables would be stored using 64-bits, and 32-bits on a 32-bit system.

Keeping the language simple, there are only very few grammatically valid statements in the language. Given that `E` could be any arbitrary expression, `S` could be any arbitrary list of statements, `N` could be any arbitrary identifier, the following are valid forms:

#### Assignments

- `define N = E` : defining a new register variable in the global scope. `N` must not be defined already.

- `set @E = E` : a store of an expression value to a computed L-value address.

- `set N = E` : a simple assignment to a register variable. `N` must be defined already.

#### Control Flow

- `if E then S else S end` : if-else statement, conditionally execute either the first block if `E` is nonzero, else execute the second block. the `else` clause is optional, which yields the valid form: `if E then S end` as well.

- `repeat S while E` : repeat-while statement, which executes the block `S` until `E` evaulates to 0.

- `do N` : An unconditional goto statement to the label `N`. 

- `at N` : Attributes the label `N` to refer to location of the next statement directly after this statement.

#### Language-Specific Constructs

- `del N` : Delete the register variable `N` from the symbol table, at this point. `N` must be defined.

- `emit E` : Emit one word of data to the executable, calculated from expression `E`.

- `eoi` : End of Input. parsing stops after this statement is encountered. Not required, but sometimes useful.

- `macro-name-here ...macro-arguments...` : Macro call. This will be explained in more detail in the Macro section of this document.

Although it is optional, End-of-input is included here to allow for commenting out large portions of code, with the use of comments, as well as allowing for documentation to be written below the users programs if they wish. 

Although there are no functions in this language, the feature of user-definable macros allow for creating effectively "always inline" functions that behave mostly as functions would, allowing for library functionality.


### Semantics of `define` and `del` statements

Although there is no concept of "scopes" in this language, The symbol tabel is able to be edited arbitrarily using the `define` and `del` statements, allowing for code such as the following:

```
(...any use of myvariable here would be an error...)

define myvariable = 5
(...can validly use myvariable here...)
del myvariable

(...any use of myvariable here would be an error...)

```

Which, has the effect of creating local scopes which can nest in arbitrary ways, some of which are not even possible to create in traditional "scope"-based programming languages. The `del` statements does not take into account control flow at all, it is purely based on the sequence of stateents interpretted as a linear sequence. 


### Syntax for Expressions

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


### Comments

As a result of this prefix-operator-based design, this means that parenthesis are never used in this language for the purposes of expressions. Thus, multi-line, nest-able comments are denoted with the following syntax:

```
(This is a comment!
    it can span multiple lines!
	(and can even contain other 
      other balanced 
 parenthesis inside!))
```

### Expression Examples

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

## Example Programs in the Language

Below are some brief examples of the language being used in practice in simple programs, to compute various mathematical results. Construction of input/output library macro-functions would allow for much greater functionality than what is shown here, eventually.


##### Example 1
```
(a simple program to loop through the numbers 0 through 15.)

define i = 0
repeat
	(...do something with i here...)
	set i = + i 1
	while < i 15
eoi
```

##### Example 2

```
(a program to compute integer-based average of an 
 array of numbers stored in memory.)

define pointer = array
define count = 7
define sum = 0
define stride = 8   (this is due to this program running on a 64-bit machine.) 

repeat
	set sum = + sum @ pointer
	set pointer = + pointer stride
	set i = + i 1
	while < i count
define average = / sum count del sum
exit 0    (<---- this is a macro from the standard library, 
		to issue an "exit" system call, lets say.)
at array
	emit 4 emit 5 emit 12 emit 2 emit 23 emit 8 emit 1
eoi

```

##### Example 3
```
(a program to compute the number of prime numbers 
 less than a given number, limit.)

define i = 0
repeat
	define j = 2
	at innerloop
		if < j i then else do prime end
		define r = % i j
		if = r 0 then do composite end del r
		set j = + j 1 del j
		do innerloop 		
at prime 
	set count = + count 1
at composite
	set i = + i 1
	while < i limit

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


