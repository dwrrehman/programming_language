//
//  preprocessor.hpp
//  language
//
//  Created by Daniel Rehman on 1901126.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef preprocessor_hpp
#define preprocessor_hpp

#include "arguments.hpp"

#include <string>
#include <vector>
#include <unordered_map>


struct macro {
    std::string pattern = "";
    std::string body = "";
};

struct preprocessed_file {
    struct file unit = {};
    std::vector<struct macro> macros = {};
};

struct preprocessed_file preprocess(struct file file);

void macro_replace(std::string pattern, std::string body, std::string &text);

#endif /* preprocessor_hpp */
