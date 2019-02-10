//
//  main.cpp
//  language
//
//  Created by Daniel Rehman on 1901104.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include <iostream>
#include <fstream>

#include "compiler.hpp"



/**
 
 
 
 
 
 
 
 
 

*/


// we need to redo this compiler.

// we need to get UD precedence for all new operations or function calls.
// x:we need no parens needlessly required in function calls.;
// we need to allow for multiple statements in a while loop or if statement.

// we need to redo the lexer to be cleaner.

// we need all operations to appear to be implemented as a function to the user.
// ie, the precendence can be changed just like anything else.
// and the user can hook from it, just like any other function.
// and the parentesis are needed for the same reason, between these ops and function calls.

// we need this langauge to be interpreteable by a "Read-Eval-Print-Loop" (REPL) interpreter.
// because making this have a playground for poeple to test stuff out in, is pretty crucial.


// header files shouldnt exist. period.
// we need a good way to seperate impl from if, but still never have a header file.

// one big idea is that files are never included, like in c or cpp.
// they are only "grouped".

// so when you import something, (eg "import .io"), a interface file will be looked for in the current working directory of where that file is if its not a stdlib module, and if it is, it will looked for in the lib directory specified on the command line.
// then, the declarations specified in the file "io.interface" in the system include dir, but they will only be accessible for the scope that included them, and its open children.
/*
 
 

 
 for example, if we have three files:
 
 - main.lang
 - module.lang
 - module.interface
 
 and in main.lang, we write:
 
    "import module"
 
 this will give us access to the decls defined in "module.interface" file.
 
 note, to actually compile, you say:
 
        >  lang-compiler .
        >  ./main
 
 and since you passed in a dir, it knows to recusrively search that dir for all ".lang" files.
 you can also pass in a different directory to search, if you want to specically search. (you can even pass in a set of dirs.)
 
 but of course, you can also just pass in the filenames manually.
 
 note: the exec name is the filename which contains (main).
 */









// x:we need to make the compiler not ignore commas completely, they should be like newlines. useful, not ignored, bu generally free to put anywhere.
// just not more than one.;


// x:we need to implement the ; in the parser.;


// we need to add support for functional function defintions.
//  ex:    "(add 4 to n) = do 4 + n"



const std::string filepath = "/Users/deniylreimn/Documents/projects/programming language/compiler/language/test.lang";

int main(int argc, const char * argv[]) {
    
    auto text = get_file(filepath);
    frontend(text);
    
    // then get the result from frontend(), and feed it into llvm, and compile stuff.
    /// next, we should work on getting the command line arguments, and be able to accept already compiled stuff, and link them together..? like in gcc.
    
    return 0;
}
