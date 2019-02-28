//
//  arguments.cpp
//  language
//
//  Created by Daniel Rehman on 1902262.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "arguments.hpp"

#include <iostream>
#include <fstream>
#include <vector>

#include "compiler.hpp"

static void open_file(struct arguments &args, struct file &file) {
    std::ifstream stream {file.name};
    if (stream.is_open()) {
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

struct arguments get_commandline_arguments(int argc, const char** argv) {
    
    struct arguments args = {};
    
    if (argc > 1 && std::string(argv[1]) == "picker") {
        args.use_interpreter = true;
        get_interpreter_arguments(argc, args, argv);
        return args;
    }
    
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "-o" && i + 1 < argc) {
            args.executable_name = std::string(argv[++i]);
        } else if (std::string(argv[i]) == "-i") {
            // inline code compilation.
        } else if (argv[i][0] == '-') {
            printf("bad option: %s\n", argv[i]);
        } else {
            struct file file = {argv[i], ""};
            open_file(args, file);
        }
    }
    return args;
}

void debug_arguments(struct arguments args) {
    for (auto a : args.files) {
        std::cout << "file: " << a.name << "\n";
        std::cout << "data: \n:::" << a.data << ":::\n";
    }
    
    std::cout << std::boolalpha;
    std::cout << "error = " << args.error << std::endl;
    std::cout << "use interpreter = " << args.use_interpreter << std::endl;
    std::cout << "exec name = " << args.executable_name << std::endl;
}
