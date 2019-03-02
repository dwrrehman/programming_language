//
//  compiler.hpp
//  language
//
//  Created by Daniel Rehman on 1901104.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef compiler_hpp
#define compiler_hpp

#include "analysis.hpp"
#include <string>
#include <vector>


struct file {
    std::string name;
    std::string data;
};

struct arguments {
    bool use_interpreter = false;
    bool error = false;
    std::vector<struct file> files = {};
    std::string executable_name = "a.out";
};

struct action_tree frontend(struct file file);
void code_generation(struct action_tree tree);

#endif /* compiler_hpp */
