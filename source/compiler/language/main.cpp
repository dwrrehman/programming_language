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
 --------------------------- todos -----------------------------------
 
 
 
 
 - redo the optional parameter parser grammar to allow for ottional parmeters in the beggmnining of the FCS.
 
 - redo the type and space interface decls in the grammar, to have a tifd have only fifd and vifd's, and timd to have actual imd stuff.
 
 - we need to do unit testing with our expression parser.
 
 
 - make files local to the file scope only, by default, unless
 
 - add an "implementing" keyword.
 
 - allow "flags" ie, "{bool which defaults to false}"
 
 - allow type aliasing with x_type := y_type
 
 - allow newlines in function definitions, type defintions, and object defintions.

 - allow newlines in function calls, etc, if the user surrounds the function call in parenthsis, etc.
 
 - allow us to put a statement in curly braces, and not have to put a required newline at the end to terminate the statement.
 
 - make "{" and "}" overridle keywords, in function signatures.
 
 - add a function qualifier, "collapses", which forces an inline for a function.
 
 - add function qualifier, "noinline", and "inline".
 
 - allow "." to be a sudonym for "const" in type expressions.
 
 - allow "~" to be a sudonym for "var" in type expressions.
 
 - allow a conditional expression to be the traditional C   " condition ? expr : expr "        (is that even possible with my lang?)
 
 - change "var" to "mut", like in rust.
 
 - make sure that in expressions, we put the array bracket index notation, AFTER the function call.
 
 
 
 
 
 
 
 ------------ analysis todos: -----------------
 
 
 - notice that you migtht only be usin a library/module in one function, and try including tha  function locally, rather than globallly. (m,inimize scope of things.)
 
 - disallow return statements in a block that is passed to a function.     (note: these are implemented as lambdas)
 
 - do type checking on the expressions,
 
 - do checking on the scope of variables, and tying their references to each other, using a symbol table.

 -
 
 
 
 
 
 
 ------------------ compiler interface todo ----------------------------
 
 - allow the user to say "nostril run main.lang"              to run the executable produced, immedately after compiling.
 
 - allow the user to say "nostril pick main.lang"              to run the code in a sandbox, and open the interpreter too.
 
 - allow the user to say "nostril --emit-llvm main.lang"       to emmit llvm and not compile the executable.
 
 - allow the user to say "nostril . --except dir/other.lang dir2/otherone.lang "     to make the recursive finding customizable.
 
 - allow the compiler to look into a directory recursively, to find all lang files.
 
 -
 
 

 we need to get UD precedence for all new operations or function calls.
 x:we need no parens needlessly required in function calls.;
 we need to allow for multiple statements in a while loop or if statement.

 
 we need all operations to appear to be implemented as a function to the user.
 ie, the precendence can be changed just like anything else.
 and the user can hook from it, just like any other function.
 and the parentesis are needed for the same reason, between these ops and function calls.

 
 we need this langauge to be interpreteable by a "Read-Eval-Print-Loop" (REPL) interpreter.
 because making this have a playground for poeple to test stuff out in, is pretty crucial.
 plottwist: the playground is the interpreter! how cool is that?

 
 header files shouldnt exist. period.
 we need a good way to seperate impl from if, but still never have a header file.

 
 one big idea is that files are never included, like in c or cpp.
 they are only "dependacy-grouped", via imports.

 
 so when you import something, (eg "import .io"), a interface file will be looked for in the current working directory of where that file is if its not a stdlib module, and if it is, it will looked for in the lib directory specified on the command line.
 then, the declarations specified in the file "io.interface" in the system include dir, but they will only be accessible for the scope that included them, and its open children.

 
 
 
 
 
 for example, if we have three files:
 
 - main.lang
 - module.lang
 - module.interface
 
 and in main.lang, we write:
 
 
 
        "import module"          or        "using module"       or        "using function in module"
 
    lets us members access the           accesses members in              just imports the member from
     namespace module, if we           namespace, and puts them           the namespace, and puts in
      reference them as "m.f"           in our current scope. "f"              our current scope.
 
 
 
 this will give us access to the decls defined in "module.interface" file.
 
 note, to actually compile, you say:
 
        dwrr:/$ lang-compiler .
        dwrr:/$ ./main
 
 and since you passed in a dir, it knows to recusrively search that dir for all ".lang" files.
 you can also pass in a different directory to search, if you want to specically search. (you can even pass in a set of dirs.)
 
 but of course, you can also just pass in the filenames manually.
 
        note: the exec name is the filename which contains "(main){...}"
 
 
 
 
 
 
 note: this compiler IS actually an REPL interpreter AS WELL, if you done pass in any command line arguments.
 
 
 
 
 
 note: if the user has a file strucure that looks like:
 
 
   Master:
     |
      - master.lang
     |
      - directory:
     |     |
     |      - mymodule.interface
     |     |
     |      - mymodule.lang
 
 
 
 and they had statements such as:
 
        file: master.lang
 
                (main) {
                    using (function a with b) in directory.mymodule
                    function 4 with 6
                }
 
 
 
        file: mymodule.interface
 
                ` a function which does a thing with two natruals numbers, and b. `
                (function d: nat with b: nat)
 
 
 
        file: mymodule.lang
 
 
 
                (function a: nat with b: nat) {
                    using .io
                    print "\{a} + \(b)"
                }
 
 
 
 this line of code would look for the file "mymodule.interface", in "directory", and add the function "(function _ with _)", (assuming that parameters are located at the underscores,)
 
  and then the bash command to compile the project would simply be:       (if our PWD is "Master/..")
 
 
            dwrr:/$ lang-compiler Master
            dwrr:/$ ./master
            4 6
            dwrr:/$
 
 
 

 
 */


const std::string filepath = "/Users/deniylreimn/Documents/projects/programming language/source/compiler/language/test.lang";

int main(int argc, const char * argv[]) {
    
    // arguments = get_commandline_arguments(argc, argv);
    auto text = get_file(filepath); // temp
    // do_something_based_on(arguments);
    frontend(text); // tree = frontend();
    // backend(tree);
    // link, or do something now...
    
    return 0;
}
