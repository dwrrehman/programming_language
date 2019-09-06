//
//  arguments.cpp
//  language
//
//  Created by Daniel Rehman on 1902262.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "arguments.hpp"

#include "lists.hpp"
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>


//TODO: make the struct file contain a list of flags, which are
// values from an enum class, and the parser, corrector, etc,
// looks at these flags to determine the correct behavior/debug info to give.
// also, this must be completely done in the get cli args function,
// and not change any external interfaces, only extent interfaces.


static void open_file(struct arguments &args, struct file &file) {
    std::ifstream stream {file.name};
    if (stream.good()) {
        std::string text {std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
        file.text = text;
        stream.close();
        args.files.push_back(file);
    } else {
        printf("Unable to open \"%s\" \n", file.name.c_str());
        perror("open");
        args.error = true;
    }
}

static void get_interpreter_arguments(int argc, struct arguments &args, const char **argv) {
    if (argc > 2) {
        struct file file = {argv[2], ""};
        open_file(args, file);
    } else if (argc == 2) {
        struct file file = {"()", ""};
        args.files.push_back(file);
    } else {
        args.error = true;
        printf("pick: error: too many files passed, only 1 allowed\n");
    }
}

int is_file(const char *path) {
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

void error_cannot_open(struct arguments &args, struct file &file) {
    printf("Unable to open \"%s\" \n", file.name.c_str());
    perror("open");
    args.error = true;
}


/*
 
    ---------------- specifcation of cli interface ------------------------

 
 nostril                        : open the REPL interpreter.
 
 nostril <.n, ...>              : compile the following files, into a executable.

    allowed file extensions:
    
        .n      n3zqx2l file

    passed to the linker:

        .o      object file

        .a      statically linked library

        ...and anyting else that the linker accepts.


 
 nostril pick                   : run the picker editor, with an empty temp file.
 
 nostril pick <.n>              : run the picker editor.
 
 nostril run <args>             : compile a nostril project, and then run it,

 nostril pack                   : create a package, instead of an executable.
 
 ------------------------------ flags ------------------------------
 
 nostril -empty                  : dont include the standard library implicitly.
 
 nostril <.a/.np>                 : a archive file to statically link.
 
 nostril -named <name>           : name the executable to the name.
 
 nostril -sneeze                 : enable debug output of the compiler, while compiling.
 
 nostril -color=<ascii/256/...>  : toggle the colors of the compiler output, or interpreter.



 nostril -indent <positive number>       : define the number of spaces per indent.
 
 

 
 */

void print_usage() {
    std::cout << "(nostril: an n3zqx2l compiler)\n";
    
    std::cout << "modes:\n";
    std::cout << "\tpick <.n>      :: open REPL editor with file. \n";
    std::cout << "\trun <sources>  :: JIT compile sources.\n";
    std::cout << "\tusage(-h)      :: print this usage dialog.\n";
    std::cout << "\tversion(-v)    :: print the compiler and language version. \n\n";
    
    std::cout << "options:\n";
    std::cout << "\t-named(-o) <executable name>  :: set the name of the executable.\n";    
    std::cout << "\t-indent <integer>   :: set the number of spaces counted as an indent. \n";
    std::cout << "\t-sneeze             :: enable the compilers debug mode. \n\n";
    std::cout << "\t-empty <.n>             :: disable implicit inclusion of _core for the file. \n\n";
}

struct arguments get_commandline_arguments(const int argc, const char** argv) {    

    //TODO: redo this function, to parse the modes first, then parse the options for each mode.
    // then parse all the files.
    // we should be able to put options in the different modes, and still have it recognize the correct number files correctly. (ie, not based on argc counts. (because there could be arguments.))

    struct arguments args = {};
    
    if (argc > 1 and std::string(argv[1]) == "pick") {
        args.use_interpreter = true;
        get_interpreter_arguments(argc, args, argv);
        return args;
        
    } else if (argc > 1 and std::string(argv[1]) == "run") { 
        args.use_interpreter = true;   

    } else if (argc > 1 and (std::string(argv[1]) == "version" or std::string(argv[1]) == "-v")) {
        std::cout << language_name << ": " << language_version << std::endl;
        //std::cout << compiler_name << ": " << compiler_version << std::endl;
        exit(0);
        
    } else if (argc > 1 and (std::string(argv[1]) == "usage" or std::string(argv[1]) == "-h")) {
        print_usage();
        exit(0);
    
    } else if (argc == 1) {        
        args.use_repl = true;
        return args;
    }
    
    for (int i = 1; i < argc; i++) {
        if ((std::string(argv[i]) == "-named" or std::string(argv[i]) == "-o") and i + 1 < argc) {
            args.executable_name = std::string(argv[++i]);

        } else if (std::string(argv[i]) == "-indent" and i + 1 < argc) {
            spaces_count_for_indent = atoi(argv[++i]);
            if (!spaces_count_for_indent) {
                printf("error: invalid number for indent width, using the default of 4 spaces.\n");
                spaces_count_for_indent = 4;
            }

        } else if (std::string(argv[i]) == "-sneeze") {
            debug = true;
            
        } else if (std::string(argv[i]) == "-empty") {
            args.include_core = false;
                        
            
        } else if (argv[i][0] == '-') {
            std::cout << "bad option: " << argv[i] << "\n";
            print_usage();
            args.error = true;
            return args;
            
        } else if (std::string(argv[i]) == "run") {
            continue;
            
        } else {
            struct file file = {argv[i], ""};
            if (is_file(argv[i])) {
                open_file(args, file);
            } else {
                error_cannot_open(args, file); 
            }
        }
    }
    return args;
}
