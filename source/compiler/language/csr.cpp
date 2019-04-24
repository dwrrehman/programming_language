//
//  csr.cpp
//  language
//
//  Created by Daniel Rehman on 1904232.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "csr.hpp"
#include "analysis.hpp"
#include "parser.hpp"
#include "nodes.hpp"
#include "lists.hpp"

#include <iostream>
#include <vector>
#include <stdlib.h>
#include <sstream>
#include <algorithm>

static int pointer = 0;
int pointer_save() {return pointer;}
void pointer_revert(int saved) {pointer = saved;}


static signature csr_solver(const std::vector<signature> list, const int depth, const std::vector<std::string> given_expression) {
    int saved = pointer_save();

    for (auto signature : list) {

        struct signature parent = {};
        pointer_revert(saved);
        int e = 0;

        for (; e < signature.elements.size(); e++) {
            if (signature.elements[e].is_parameter && depth < max_expression_depth) {
                struct element result = {"", {}, true};
                result.children = csr_solver(list, depth + 1, given_expression);
                parent.elements.push_back(result);

            } else if (pointer < given_expression.size() && signature.elements[e].name == given_expression[pointer]) {
                parent.elements.push_back(signature.elements[e]);
                pointer++;

            } else if (pointer < given_expression.size()) break;
        }
        if (e == signature.elements.size()) return parent;
    }
    return {{{"Error", {}, false}}};
}

static signature fix_csr(signature s) {
    if (s.elements.size() == 1 && s.elements[0].is_parameter) return fix_csr(s.elements[0].children);
    else return s;
}

signature call_signature_resolution(const std::vector<signature> list, const std::vector<std::string> expression) {
    return fix_csr(csr_solver(list, 0, expression));
}
