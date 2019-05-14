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
#include "lists.hpp"
#include <iostream>
#include <fstream>
#include <vector>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


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


void open_dir() {

}


/*
 
    ---------------- specifcation of cli interface ------------------------

 
 nostril                        : open the REPL interpreter.
 
 nostril <.n, ...>              : compile the following files, into a executable.

    allowed file extensions:

        .n      n3zqx2l implementation file

        .ni     n3zqx2l interface file


    passed to the linker:

        .o      object file

        .a      statically linked library

        ...and anyting else that the linker accepts.


 
 nostril pick                   : run the picker editor, with an empty temp file.
 
 nostril pick <.n>              : run the picker editor.
 
 nostril run <args>             : compile a nostril project, and then run it,

 nostril pack                   : create a package, instead of an executable.
 
 ------------------------------ flags ------------------------------
 
 nostril -void                  : dont include the standard library implicitly.
 
 nostril <.a/.np>                 : a archive file to statically link.
 
 nostril -named <name>           : name the executable to the name.
 
 nostril -sneeze                 : enable debug output of the compiler, while compiling.
 
 nostril -color=<ascii/256/...>  : toggle the colors of the compiler output, or interpreter.



 nostril -indent <positive number>       : defien the number of spaces per indent.
 
 */


struct arguments get_commandline_arguments(const int argc, const char** argv) {
    // we need to revise this function to fit the new comipiler specfication.

    //TODO: redo this function, to parse the modes first, then parse the options for each mode.
    // then parse all the files.
    // we should be able to put options in the different modes, and still have it recognize the correct number files correctly. (ie, not based on argc counts. (because there could be arguments.))

    struct arguments args = {};
    
    if (argc > 1 and std::string(argv[1]) == "pick") {
        args.use_interpreter = true;
        get_interpreter_arguments(argc, args, argv);
        return args;
        
    } else if (argc > 1 and std::string(argv[1]) == "interpret") { // "nostril interpret"
        
    } else if (argc > 1 and std::string(argv[1]) == "run") { // "nostril run ..."
        
    } else if (argc > 1 and std::string(argv[1]) == "compile") { // "nostril compile ..."

    } else if (argc > 1 and std::string(argv[1]) == "entry") { // "nostril entry ..."
        
    } else if (argc == 1) {
        args.use_interpreter = true;
        args.files.push_back({"{repl}", ""});
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

        } else if (std::string(argv[i]) == "-version" or std::string(argv[i]) == "-v") {
            std::cout << language_name << ": " << language_version << std::endl;
            exit(0);

        } else if (std::string(argv[i]) == "") {


        } else if (argv[i][0] == '-') {
            printf("bad option: %s\n", argv[i]);
            // print_usage();
            
        } else {
            struct file file = {argv[i], ""};
            if (is_file(argv[i])) {
                open_file(args, file);
            } else {
                open_dir(); // TODO: fill in the recursive compiler.
            }
        }
    }
    return args;
}
