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





/// ---------- used for type checking. -------------------


bool signatures_are_equal(signature a, signature b);
bool elements_are_equal(struct element a, struct element b);

bool elements_are_equal(struct element a, struct element b) {
    return (
        a.is_parameter == b.is_parameter &&
        a.name == b.name &&
        signatures_are_equal(a.type, b.type)
    );
}

bool signatures_are_equal(signature a, signature b) {
    if (a.elements.size() != b.elements.size()) return false;
    for (size_t i = 0; i < a.elements.size(); i++) {
        if (!elements_are_equal(a.elements[i], b.elements[i])) {
            return false;
        }
    }
    return true;
}

/// -------------------------------------------------------







signature csr(const std::vector<signature> list, const int depth, const std::vector<std::string> given_expression) {

    if (depth >= max_expression_depth) return {{{"_erroneous", {}, false}}};
    int saved = pointer_save();
    for (auto signature : list) {
        struct signature solution = {};
        pointer_revert(saved);
        bool failed = false;
        for (auto element : signature.elements) {
            if (element.is_parameter && depth < max_expression_depth) {
                struct element result = {"", csr(list, depth + 1, given_expression), true};
                if (result.children.elements[0].name == "_erroneous") { failed = true; break; }
                solution.elements.push_back(result);
            } else if (pointer < given_expression.size() && element.name == given_expression[pointer]) {
                solution.elements.push_back(element);
                pointer++;
            } else { failed = true; break; }
        }
        if (!failed) return solution;
    }

    // for undefined symbols, it should go right here.
    
    /// push current se onto basket.

    csr(list, depth, given_expression);

    return {{{"_erroneous", {}, false}}};
}



