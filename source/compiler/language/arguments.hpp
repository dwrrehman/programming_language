//
//  arguments.hpp
//  language
//
//  Created by Daniel Rehman on 1902262.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef arguments_hpp
#define arguments_hpp

#include <iostream>
#include <fstream>
#include <vector>

struct file {
    std::string name = "";
    std::string text = "";
    bool is_main = false;
};

struct arguments {    
    std::vector<struct file> files = {};
    std::string executable_name = "a.out";
    bool use_repl = false;
    bool include_core = true;
    bool use_interpreter = false;    
    bool error = false;
};

struct arguments get_commandline_arguments(const int argc, const char** argv);
void debug_arguments(struct arguments args);

#endif /* arguments_hpp */
