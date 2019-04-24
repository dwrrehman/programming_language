//
//  csr.hpp
//  language
//
//  Created by Daniel Rehman on 1904232.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef csr_hpp
#define csr_hpp

#include "analysis.hpp"
#include "parser.hpp"
#include "nodes.hpp"
#include "lists.hpp"

#include <iostream>
#include <vector>
#include <stdlib.h>
#include <sstream>
#include <algorithm>

struct element;
struct signature {
    std::vector<struct element> elements;
};
struct element {
    std::string name = "";
    signature children = {};

    bool is_parameter = false;
    signature type = {};
};

signature call_signature_resolution(const std::vector<signature> list, const std::vector<std::string> expression);

#endif /* csr_hpp */
