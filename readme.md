# A Programming Langauge Created By Daniel Rehman.

this is a programming lang. its awesome.


### hello world program:

```
1 (main) {
2    using .io
3    print "Hello, world!"
4 }
```

To get a feeling of how this language works, ill explain this program.
the line 1 of the main program is just declaring a function "main", which takes no parameters, and returns "nothing" (aka, "()"). 

line 2 is called a "using statement", and functions to not only allow the system library ".io" itself to be accessed by the current scope by including it,
but it also works to allow all of the immediate members in the .io system library to be accessed in the current scope as well, without saying ".io.print". 
if you didnt want all the immediate members of the library to be included in the current scope, you would say "import .io". 
the "." infront of the filename, means its a system library. note that dependancies are tied to definitions, not to files, quite like the D programming language.

line 3 is the actual call to "print", to print something to the screen. the function automatically prints a newline by default, however, there are optional parameters 
you can give to make it not print any terminator. note that "print" is not a keyword, (even though it may look like it, coming from as language like Python), 
Its actually a function call. this language utilizes a new function call syntax i devised, which is much more intuitive then the traditional widely used "C" function call syntax.
thus, line 3 is actually passing in a string (which arent built into the language, but provided by including by the .io library) into the function print.
no need to terminate a lines with semicolons, as newlines are sufficent.

line 4 is your programmatic, warm, familar end curly brace, which terminates main, which will implicitly return 0.

...and thats the hello world program!



