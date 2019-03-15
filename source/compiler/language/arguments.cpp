//
//  arguments.cpp
//  language
//
//  Created by Daniel Rehman on 1902262.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "arguments.hpp"
#include "compiler.hpp"
#include "debug.hpp"

#include <iostream>
#include <fstream>
#include <vector>


static void open_file(struct arguments &args, struct file &file) {
    std::ifstream stream {file.name};
    if (stream.good()) {
        std::string text {std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
        file.data = text;
        stream.close();
        args.files.push_back(file);
    } else {
        printf("Unable to open \"%s\" \n", file.name.c_str());
        args.error = true;
    }
}

static void get_interpreter_arguments(int argc, struct arguments &args, const char **argv) {
    if (argc > 2) {
        struct file file = {argv[2], ""};
        open_file(args, file);
    } else if (argc == 2) {
        struct file file = {"", ""};
        args.files.push_back(file);
    } else {
        args.error = true;
        printf("Too many files passed to the interpreter\n");
    }
}



/*
 
    ---------------- specifcation of cli interface ------------------------

 
 nostril                        : open the REPL interpreter.
 
 nostril <.n, ...>              : compile the following files, into a executable.
 
 nostril interpret <s>          : run the interpreter on a given string nostril statment, s
 
 nostril pick                   : run the picker editor, with an empty temp file.
 
 nostril pick <.n>              : run the picker editor.

 nostril compile <project>      : compile a nostril project in the current directory.
 
 nostril run <args>             : compile a nostril project, and then run it,
 
 ------------------------------ flags ------------------------------
 
 nostril -empty                  : dont include the standard library implicitly.
 
 nostril -link <.a/.bgr>         : a archive file, or a booger to statically link.
 
 nostril -named <name>           : name the executable to the name.
 
 nostril -sneeze                 : enable debug output of the compiler, while compiling.
 
 nostril -booger                 : create a ".bgr" package, instead of an executable.
 
 nostril -color=<ascii/256/...>  : toggle the colors of the compiler output, or interpreter.

 nostril -entry <one file>       : define this file as the entry point, and wrap a anon lambda around the code.
 
 */
struct arguments get_commandline_arguments(int argc, const char** argv) { // we need to revise this function to fit the new comipiler specfication.
    
    struct arguments args = {};
    
    if (argc > 1 && std::string(argv[1]) == "pick") {
        args.use_interpreter = true;
        get_interpreter_arguments(argc, args, argv);
        return args;
        
    } else if (argc > 1 && std::string(argv[1]) == "interpret") { // "nostril interpret"
        
    } else if (argc > 1 && std::string(argv[1]) == "run") { // "nostril run ..."
        
    } else if (argc > 1 && std::string(argv[1]) == "compile") { // "nostril compile ..."
        
    } else if (argc == 1) {
        
    }
    
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "-named" && i + 1 < argc) {
            args.executable_name = std::string(argv[++i]);
            
        } else if (std::string(argv[i]) == "-n") {
            // ?
            
        } else if (argv[i][0] == '-') {
            printf("bad option: %s\n", argv[i]);
            
        } else {
            struct file file = {argv[i], ""};
            open_file(args, file);
        }
    }
    return args;
}
