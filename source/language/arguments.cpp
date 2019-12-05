#include "arguments.hpp"

#include "lists.hpp"
#include "error.hpp"
#include "debug.hpp"

#include <iostream>
#include <fstream>

static inline void open_file(std::string filename, arguments& args) {
    std::ifstream stream {filename};
    if (stream.good()) {
        std::string text {std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
        stream.close();
        args.files.push_back({filename, text});
    } else {
        printf("Unable to open \"%s\" \n", filename.c_str());
        perror("open");
        exit(1);
    }
}

static inline void print_usage() {
    printf("./nostril -[uv] [run] -[sdoei]\n");
    printf("./nostril -version\n");
    printf("./nostril run ...\n");
    exit(0);
}

static inline void print_version() {
    std::cout << language_name << ": " << language_version << std::endl;
    std::cout << compiler_name << ": " << compiler_version << std::endl;
    exit(0);
}

arguments get_commandline_arguments(const int argc, const char** argv) {
    arguments args = {};
    if (argc == 1) exit(1);
    const auto first = std::string(argv[1]);
    if (first == "-u") print_usage();
    else if (first == "-v") print_version();
    else if (first == "run") args.interpret = true;
    
    for (int i = 1 + args.interpret; i < argc; i++) {
        const auto word = std::string(argv[i]);
        if (word == "-s") debug = true;
        else if (word == "-e") args.empty = true;
        else if (word == "-o" and i + 1 < argc) args.executable_name = argv[++i];
        else if (word == "-i" and i + 1 < argc) { auto n = atoi(argv[++i]); spaces_count_for_indent = n ? n : 4; }
        else if (word == "-d" and i + 1 < argc) { auto n = atoi(argv[++i]); max_expression_depth = n ? n : 4; }
        else if (word[0] == '-') { printf("bad option: %s\n", argv[i]); exit(1); }
        else open_file(word, args);
    }
    if (debug) debug_arguments(args);
    return args;
}
